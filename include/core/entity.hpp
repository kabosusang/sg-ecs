#pragma once
#include <compare>

#include <config.hpp>

namespace ecs {

struct entity {
	//entity type
	using entity_id_type = ENTITY_ID_TYPE;
	//[index,version]
	entity_id_type entity_;

	//index bits
	static constexpr entity_id_type INDEX_BITS = ENTITY_INDEX_BITS;
	//version bits
	static constexpr entity_id_type VERSION_BITS = sizeof(entity_id_type) * 8 - INDEX_BITS;
	
	static constexpr entity_id_type INDEX_MASK = (entity_id_type(1) << INDEX_BITS) - 1;
	static constexpr entity_id_type VERSION_MASK = ~INDEX_MASK;

	constexpr entity() : entity_(0) {}
	constexpr entity(entity_id_type index, entity_id_type version) : entity_((version << INDEX_BITS) | (index & INDEX_MASK)) {}
	explicit constexpr entity(entity_id_type raw) : entity_(raw) {}

	//get
	[[nodiscard]] constexpr entity_id_type index() const { return entity_ & INDEX_MASK; }
	[[nodiscard]] constexpr entity_id_type version() const { return entity_ >> INDEX_BITS; }
	[[nodiscard]] constexpr entity_id_type raw() const { return entity_; }
	//operator entity_id_type()
	constexpr operator entity_id_type() const { return index(); }
	//operator
	[[nodiscard]] constexpr auto operator<=>(entity o) const noexcept {
		return entity_ <=> o.entity_;
	}
};

} //namespace ecs
