#pragma once
#include <flat_set>

#include "core/component_mask.hpp"

namespace ecs {

struct archetype {
	size_t id = SIZE_MAX;
	component_mask table_mask;
	std::flat_set<component_id_type> table_comps;
	table table;

	[[nodiscard]] bool has(component_id_type id) const noexcept {
		return table_mask.test(id);
	}
};






    

} //namespace ecs
