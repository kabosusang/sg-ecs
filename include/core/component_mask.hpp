#pragma once
#include <assert.h>

#include "component.hpp"

namespace ecs {

class component_mask {
	uint64_t mask_ = 0;

public:
	component_mask() = default;

	void set(component_id_type id) noexcept {
		assert(id < 64 && "component_mask: max 64 components");
		mask_ |= (1ull << id);
	}
	void reset(component_id_type id) noexcept {
		mask_ &= ~(1ull << id);
	}
	[[nodiscard]] bool test(component_id_type id) const noexcept {
		return (mask_ >> id) & 1;
	}

	[[nodiscard]] bool contains_all(const component_mask& other)
			const noexcept {
		return (mask_ & other.mask_) == other.mask_;
	}
	[[nodiscard]] bool intersects(const component_mask& other)
			const noexcept {
		return mask_ & other.mask_;
	}
	[[nodiscard]] bool empty() const noexcept {
		return mask_ == 0;
	}

	[[nodiscard]] bool operator==(const component_mask& other) const noexcept {
		return mask_ == other.mask_;
	}

	[[nodiscard]] bool operator!=(const component_mask& other) const noexcept {
		return mask_ != other.mask_;
	}

	//遍历
	template <typename F>
		requires std::invocable<F, component_id_type>
	void for_each(F&& cb) const {
		uint64_t bits = mask_;
		while (bits) {
			cb(static_cast<component_id_type>(std::countr_zero(bits)));
			bits &= bits - 1;
		}
	}
};
} //namespace ecs
