#pragma once
#include "tick.hpp"

namespace ecs {

struct resource_tick {
	tick added_ = TICK_NONE; //  什么时候添加的
	tick changed_ = TICK_NONE; //什么时候改变的
};

//这里其实模仿的是rust 的&T 和 &mut T haha
template <typename T>
class ref {
	const T* ptr_;

public:
	explicit ref(const T* p) noexcept : ptr_(p) {}
	const T& operator*() const noexcept { return *ptr_; }
	const T* operator->() const noexcept { return ptr_; }
};

template <typename T>
class mut {
	T* ptr_;
	resource_tick* tick_;
	tick current_tick_;

	mut(T* p, resource_tick* t, tick ct) noexcept : ptr_(p), tick_(t), current_tick_(ct) {}
	T& operator*() noexcept {
		tick_->changed_ = current_tick_;
		return *ptr_;
	}
	T* operator->() noexcept {
		tick_->changed_ = current_tick_;
		return ptr_;
	}
	[[nodiscard]] T& get() noexcept { return operator*(); }
	[[nodiscard]] T* raw() noexcept { return ptr_; }
};

} //namespace ecs
