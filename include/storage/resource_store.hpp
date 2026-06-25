#pragma once
#include <vector>

#include "blob_array.hpp"
#include "core/change_detection.hpp"
#include "core/component.hpp"

namespace ecs {

class resource_store {
	struct resource {
		void* data_ = nullptr;
		resource_tick tick_;
	};

public:
	template <typename T>
	void insert(T&& value) {
		using type = std::decay_t<T>;
		component_id_type cid = resource_type_id<type>();
		if (cid >= entries_.size()) {
			entries_.resize(cid + 1);
		}
		auto& e = entries_[cid];
		if (e.data_) {
			blob_array_accessor<type>::drop(e.data_);
			::operator delete(e.data_);
		}
		void* mem = ::operator new(sizeof(type), std::align_val_t{ alignof(type) });
		::new (mem) type(std::forward<T>(value));
		e.data_ = mem;
		e.tick_.added_ = TICK_NONE;
		e.tick_.changed_ = TICK_NONE;
	}

	template <typename T>
	[[nodiscard]] T* get() noexcept {
		component_id_type cid = resource_type_id<T>();
		if (cid < entries_.size() && entries_[cid].data_) {
			return static_cast<T*>(entries_[cid].data_);
		}
		return nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* get() const noexcept {
		component_id_type cid = resource_type_id<T>();
		if (cid < entries_.size() && entries_[cid].data_) {
			return static_cast<const T*>(entries_[cid].data_);
		}
		return nullptr;
	}

	template <typename T>
	[[nodiscard]] bool exists() const noexcept {
		component_id_type cid = resource_type_id<T>();
		return cid < entries_.size() && entries_[cid].data_ != nullptr;
	}

	template <typename T>
	void remove() noexcept {
		component_id_type cid = resource_type_id<T>();
		if (cid >= entries_.size() || !entries_[cid].data_) {
			return;
		}
		blob_array_accessor<T>::drop(entries_[cid].data_);
		::operator delete(entries_[cid].data_);
		entries_[cid].data_ = nullptr;
	}

	template <typename T>
	[[nodiscard]] resource_tick* tick() noexcept {
		component_id_type cid = resource_type_id<T>();
		if (cid < entries_.size() && entries_[cid].data_) {
			return &entries_[cid].tick_;
		}
		return nullptr;
	}

	void mark_changed(component_id_type cid, ecs::tick ct) noexcept {
		if (cid < entries_.size() && entries_[cid].data_) {
			entries_[cid].tick_.changed_ = ct;
		}
	}

private:
	std::vector<resource> entries_;
};

// ---- res: 只读资源引用 ----
template <typename T>
class res {
public:
	res(resource_store& store) noexcept : ptr_(store.get<T>()) {}

	[[nodiscard]] const T& operator*() const noexcept { return *ptr_; }
	[[nodiscard]] const T* operator->() const noexcept { return ptr_; }
	[[nodiscard]] const T* raw() const noexcept { return ptr_; }

private:
	const T* ptr_;
};

// ---- res_mut: 可写资源引用（自动变更检测） ----
template <typename T>
class res_mut {
public:
	res_mut(resource_store& store, tick current_tick) noexcept
			: ptr_(store.get<T>()),
			  tick_(store.tick<T>()),
			  current_tick_(current_tick) {}

	[[nodiscard]] T& operator*() noexcept {
		if (tick_) {
			tick_->changed_ = current_tick_;
		}
		return *ptr_;
	}

	[[nodiscard]] T* operator->() noexcept {
		if (tick_) {
			tick_->changed_ = current_tick_;
		}
		return ptr_;
	}

	[[nodiscard]] T* raw() noexcept { return ptr_; }

private:
	T* ptr_;
	resource_tick* tick_;
	tick current_tick_;
};

} //namespace ecs
