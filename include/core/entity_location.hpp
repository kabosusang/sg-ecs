#pragma once
#include <cstddef>
#include <limits>

namespace ecs {

constexpr size_t INVALID_ROW = std::numeric_limits<size_t>::max();


struct entity_location {
	size_t archetype_id = INVALID_ROW; // 指向哪个 Archetype/Table
	size_t table_row = INVALID_ROW; // 在 Table 中的行号(entity实体)
	[[nodiscard]] bool valid() const noexcept {
		return archetype_id != INVALID_ROW && table_row != INVALID_ROW;
	}
};

/*
Archetype: {Position, Velocity}
	↓ 对应
Table:
┌──────┬──────────┬───────────┐
│ Row  │ Position │ Velocity  │
├──────┼──────────┼───────────┤
│  0   │ (0,0,0)  │ (1,0,0)   │  ← 实体A
│  1   │ (5,2,1)  │ (0,1,0)   │  ← 实体B
│  2   │ (3,1,4)  │ (2,0,1)   │  ← 实体C
└──────┴──────────┴───────────┘
0 1 2 -> table_row
*/

} //namespace ecs
