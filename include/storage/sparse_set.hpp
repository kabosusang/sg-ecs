#pragma once

#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "blob_array.hpp"
#include "core/entity.hpp"

namespace ecs {

class raw_sparse_set {
	static constexpr uint32_t NONE = std::numeric_limits<uint32_t>::max();
	std::vector<uint32_t> sparse_;
	std::vector<entity> entities_;
	blob_array dense_;

public:
	raw_sparse_set(size_t item_size, size_t item_align,
			blob_array::drop_fn drop, blob_array::move_fn move) : dense_(item_size, item_align, drop, move) {}

	void insert(entity e, const void* value) {
		if (e >= sparse_.size()) [[unlikely]] {
			sparse_.resize(e + 1, NONE);
		}
		uint32_t& slot = sparse_[e];
		if (slot != NONE) {
			dense_.set(slot, value); 
			return;
		}
		slot = static_cast<uint32_t>(dense_.size());
		dense_.push_back(value);
		entities_.push_back(e);
	}

	void* get(entity e) {
		if (e >= sparse_.size()) [[unlikely]] {
			return nullptr;
		}
		uint32_t idx = sparse_[e];
		return idx != NONE ? dense_.at(idx) : nullptr;
	}

	const void* get(entity e) const {
		if (e >= sparse_.size()) [[unlikely]] {
			return nullptr;
		}
		uint32_t idx = sparse_[e];
		return idx != NONE ? dense_.at(idx) : nullptr;
	}

	bool contains(entity e) const {
		return e < sparse_.size() && sparse_[e] != NONE;
	}

	void remove(entity e) {
		if (e >= sparse_.size() || sparse_[e] == NONE) {
			return;
		}
		uint32_t idx = sparse_[e];
		uint32_t last = static_cast<uint32_t>(dense_.size()) - 1;
		if (idx != last) {
			dense_.set(idx, dense_.at(last));
			entities_[idx] = entities_[last];
			sparse_[entities_[last]] = idx;
		}
		dense_.pop_back();
		entities_.pop_back();
		sparse_[e] = NONE;
	}

	size_t size() const noexcept { return dense_.size(); }
	bool empty() const noexcept { return dense_.empty(); }
	size_t item_size() const noexcept { return dense_.item_size(); }

	void* data() noexcept { return dense_.data(); }
	const void* data() const noexcept { return dense_.data(); }

	std::span<entity> entity_list() noexcept { return entities_; }
	std::span<const entity> entity_list() const noexcept { return entities_; }
};



template <typename T>
class sparse_set {
	static constexpr uint32_t NONE = std::numeric_limits<uint32_t>::max();
	std::vector<uint32_t> sparse_; //记录数据在dense_的位置
	std::vector<entity> entities_; //记录实体数据
	std::vector<T> dense_; //value
	// insert(3,100)
	// sparse_: [none,none,none,3]
	// entities_:[3]
	// dense_: [100]
public:
	void insert(entity e, T value) noexcept {
		if (e >= sparse_.size()) [[unlikely]] {
			sparse_.resize(e + 1, NONE);
		}

		auto& slot = sparse_[e];
		if (slot != NONE) {
			dense_[slot] = std::move(value);
			return;
		}

		slot = static_cast<uint32_t>(dense_.size());
		dense_.push_back(std::move(value));
		entities_.push_back(e);
	}

	T* get(entity e) noexcept {
		if (e >= sparse_.size()) [[unlikely]] {
			return nullptr;
		}
		uint32_t idx = sparse_[e];
		return idx != NONE ? &dense_[idx] : nullptr;
	}

	const T* get(entity e) const noexcept {
		if (e >= sparse_.size()) [[unlikely]] {
			return nullptr;
		}
		uint32_t idx = sparse_[e];
		return idx != NONE ? &dense_[idx] : nullptr;
	}

	[[nodiscard]] bool contains(entity e) const noexcept {
		return e < sparse_.size() && sparse_[e] != NONE;
	}

	void remove(entity e) noexcept {
		if (e >= sparse_.size() || sparse_[e] == NONE) [[unlikely]] {
			return;
		}
		uint32_t idx = sparse_[e];
		uint32_t last = static_cast<uint32_t>(dense_.size()) - 1;
		if (idx != last) {
			dense_[idx] = std::move(dense_[last]);
			entities_[idx] = entities_[last];
			sparse_[entities_[last]] = idx;
		}
		dense_.pop_back();
		entities_.pop_back();
		sparse_[e] = NONE;
	}

	[[nodiscard]] size_t size() const noexcept { return dense_.size(); }
	[[nodiscard]] bool empty() const noexcept { return dense_.empty(); }

	T* data() noexcept { return dense_.data(); }
	const T* data() const noexcept { return dense_.data(); }

	std::span<T> items() noexcept { return dense_; }
	std::span<const T> items() const noexcept { return dense_; }

	std::span<entity> entity_list() noexcept { return entities_; }
	std::span<const entity> entity_list() const noexcept { return entities_; }
};

} // namespace ecs
