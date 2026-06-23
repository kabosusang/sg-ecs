#pragma once
#include <memory>

#include <core/component_registry.hpp>
#include <core/entity_index.hpp>
#include <storage/archetype.hpp>
#include <storage/sparse_set.hpp>

namespace ecs {

class world {
public:
	world() {
		archetypes_.emplace_back(new archetype(0));
	}

	// ================ Entity ==========================

	[[nodiscard]] entity spawn() {
		auto e = entities_.spawn();
		auto& root = *archetypes_[0];
		auto row = root.table_.push_row(e);
		entities_.location(e) = { 0, row };
		return e;
	}

	void despawn(entity e) {
		if (!is_alive(e)) [[unlikely]] {
			return;
		}

		auto& loc = entities_.location(e);
		auto& arch = *archetypes_[loc.archetype_id];

		arch.table_.swap_remove(loc.table_row, [this](entity moved, size_t new_row) {
			entities_.location(moved).table_row = new_row;
		});

		entities_.despawn(e);
	}

	[[nodiscard]] bool is_alive(entity e) const noexcept {
		return entities_.is_alive(e);
	}

	[[nodiscard]] size_t entity_count() const noexcept {
		return entities_.entity_count();
	}

	void clear_entities() {
		for (auto& arch : archetypes_) {
			while (arch->table_.row_count() > 0) {
				arch->table_.swap_remove(0, [this](entity moved, size_t new_row) {
					entities_.location(moved).table_row = new_row;
				});
			}
		}
		entities_ = entity_index{};
	}

	// ================ Component =======================

	template <typename T>
	void register_component(std::string name, storage_type st = storage_type::table) {
		registry_.register_component<T>(std::move(name), st);

		if (st == storage_type::sparse_set) {
			component_id_type cid = type_index<T>();
			if (cid >= sparse_sets_.size()) {
				sparse_sets_.resize(cid + 1);
			}
			if (!sparse_sets_[cid]) {
				sparse_sets_[cid] = std::make_unique<raw_sparse_set>(
						sizeof(T),
						alignof(T),
						blob_array_accessor<T>::drop,
						blob_array_accessor<T>::move_construct);
			}
		}
	}

	// ================ Table Component =================

	template <typename T>
	[[nodiscard]] T* get_component(entity e) noexcept {
		if (!is_alive(e)) {
			return nullptr;
		}

		auto& loc = entities_.location(e);
		auto& arch = *archetypes_[loc.archetype_id];

		if (!arch.table_mask_.test(type_index<T>())) {
			return nullptr;
		}
		return &arch.table_.template at<T>(loc.table_row);
	}

	template <typename T>
	T& add_component(entity e, T&& value) {
		assert(is_alive(e));
		component_id_type cid = type_index<T>();
		auto& loc = entities_.location(e);
		auto& from = *archetypes_[loc.archetype_id];

		// 原型已有该组件，原地更新
		if (from.table_mask_.test(cid)) {
			auto& col = from.table_.template column_blob<T>();
			blob_array_accessor<T>::get(col, loc.table_row) = std::forward<T>(value);
			return blob_array_accessor<T>::get(col, loc.table_row);
		}

		// 迁移到新原型
		size_t to_id = get_or_create_arch(loc.archetype_id, cid, from);
		auto& to = *archetypes_[to_id];

		to.table_.ensure_column_by_id(cid, registry_);

		size_t new_row = to.table_.push_row(e);
		blob_array_accessor<T>::get(to.table_.template column_blob<T>(), new_row) = std::forward<T>(value);

		for (component_id_type fc : from.table_comps_) {
			if (fc == cid) {
				continue;
			}
			to.table_.copy_column_from(fc, from.table_, loc.table_row, new_row);
		}

		from.table_.swap_remove(loc.table_row, [this](entity moved, size_t new_row) {
			entities_.location(moved).table_row = new_row;
		});

		entities_.location(e) = { to_id, new_row };
		return blob_array_accessor<T>::get(to.table_.template column_blob<T>(), new_row);
	}

	template <typename T>
	void remove_component(entity e) {
		assert(is_alive(e));
		component_id_type cid = type_index<T>();
		auto& loc = entities_.location(e);
		auto& from = *archetypes_[loc.archetype_id];

		if (!from.table_mask_.test(cid)) {
			return;
		}

		size_t to_id = get_or_create_arch_remove(loc.archetype_id, cid, from);
		auto& to = *archetypes_[to_id];

		size_t new_row = to.table_.push_row(e);

		for (component_id_type fc : from.table_comps_) {
			if (fc == cid) {
				continue;
			}
			to.table_.copy_column_from(fc, from.table_, loc.table_row, new_row);
		}

		from.table_.swap_remove(loc.table_row, [this](entity moved, size_t new_row) {
			entities_.location(moved).table_row = new_row;
		});

		entities_.location(e) = { to_id, new_row };
	}

	// ================ SparseSet Component =============

	template <typename T>
	[[nodiscard]] T* get_sparse(entity e) noexcept {
		auto* rss = typed_sparse_set_ref<T>();
		return rss ? static_cast<T*>(rss->get(e)) : nullptr;
	}

	template <typename T>
	void set_sparse(entity e, T value) {
		auto& rss = typed_sparse_set_ensure<T>();
		rss.insert(e, &value);
	}

	template <typename T>
	[[nodiscard]] bool has_sparse(entity e) const noexcept {
		auto* rss = typed_sparse_set_ref_const<T>();
		return rss && rss->contains(e);
	}

	template <typename T>
	void remove_sparse(entity e) {
		auto* rss = typed_sparse_set_ref<T>();
		if (rss) {
			rss->remove(e);
		}
	}

	// ================ Meta ============================

	[[nodiscard]] size_t archetype_count() const noexcept { return archetypes_.size(); }
	[[nodiscard]] size_t archetype_generation() const noexcept { return archetype_generation_; }
	[[nodiscard]] const auto& archetypes() const noexcept { return archetypes_; }
	[[nodiscard]] const auto& component_index() const noexcept { return component_index_; }

private:
	// ================ Archetype Graph =================

	size_t get_or_create_arch(size_t from_id, component_id_type cid, archetype& from) {
		if (size_t cached = graph_.find(from_id, cid); cached != SIZE_MAX) {
			return cached;
		}

		component_mask target_mask = from.table_mask_;
		target_mask.set(cid);

		for (auto& arch : archetypes_) {
			if (arch->table_mask_ == target_mask &&
					arch->table_comps_.size() == from.table_comps_.size() + 1) {
				graph_.insert(from_id, cid, arch->id_);
				graph_.insert(arch->id_, cid, from_id);
				return arch->id_;
			}
		}

		auto na = std::make_unique<archetype>();
		na->id_ = archetypes_.size();
        
		na->table_mask_ = target_mask;
		na->table_comps_ = from.table_comps_;
		na->table_comps_.insert(cid);
		size_t to_id = na->id_;

		for (component_id_type fc : na->table_comps_) {
			na->table_.ensure_column_by_id(fc, registry_);
		}

		graph_.insert(from_id, cid, to_id);
		graph_.insert(to_id, cid, from_id);

		for (component_id_type fc : na->table_comps_) {
			if (fc >= component_index_.size()) {
				component_index_.resize(fc + 1);
			}
			component_index_[fc].push_back(to_id);
		}

		archetypes_.push_back(std::move(na));
		archetype_generation_ = archetypes_.size();
		return to_id;
	}

	size_t get_or_create_arch_remove(size_t from_id, component_id_type cid, archetype& from) {
		if (size_t cached = graph_.find(from_id, cid); cached != SIZE_MAX) {
			return cached;
		}

		component_mask target_mask = from.table_mask_;
		target_mask.reset(cid);

		for (auto& arch : archetypes_) {
			if (arch->table_mask_ == target_mask &&
					arch->table_comps_.size() == from.table_comps_.size() - 1) {
				graph_.insert(from_id, cid, arch->id_);
				graph_.insert(arch->id_, cid, from_id);
				return arch->id_;
			}
		}

		auto na = std::make_unique<archetype>();
		na->id_ = archetypes_.size();
		na->table_mask_ = target_mask;
		na->table_comps_ = from.table_comps_;
		na->table_comps_.erase(cid);
		size_t to_id = na->id_;

		for (component_id_type fc : na->table_comps_) {
			na->table_.ensure_column_by_id(fc, registry_);
		}

		graph_.insert(from_id, cid, to_id);
		graph_.insert(to_id, cid, from_id);

		for (component_id_type fc : na->table_comps_) {
			if (fc < component_index_.size()) {
				component_index_[fc].push_back(to_id);
			}
		}

		archetypes_.push_back(std::move(na));
		archetype_generation_ = archetypes_.size();
		return to_id;
	}
	// ================ SparseSet Helper ================


	template <typename T>
	raw_sparse_set* typed_sparse_set_ref() noexcept {
		component_id_type cid = type_index<T>();
		if (cid >= sparse_sets_.size() || !sparse_sets_[cid]) {
			return nullptr;
		}
		return sparse_sets_[cid].get();
	}

	template <typename T>
	const raw_sparse_set* typed_sparse_set_ref_const() const noexcept {
		component_id_type cid = type_index<T>();
		if (cid >= sparse_sets_.size() || !sparse_sets_[cid]) {
			return nullptr;
		}
		return sparse_sets_[cid].get();
	}

	template <typename T>
	raw_sparse_set& typed_sparse_set_ensure() {
		component_id_type cid = type_index<T>();
		if (cid >= sparse_sets_.size()) {
			sparse_sets_.resize(cid + 1);
		}
		if (!sparse_sets_[cid]) {
			sparse_sets_[cid] = std::make_unique<raw_sparse_set>(
					sizeof(T), alignof(T),
					blob_array_accessor<T>::drop,
					blob_array_accessor<T>::move_construct);
		}
		return *sparse_sets_[cid];
	}

private:
    
	entity_index entities_;
	component_registry registry_;

	archetype_graph graph_;
	std::vector<std::unique_ptr<archetype>> archetypes_;
	std::vector<std::vector<size_t>> component_index_;
	size_t archetype_generation_ = 0;

	std::vector<std::unique_ptr<raw_sparse_set>> sparse_sets_;
};

} //namespace ecs
