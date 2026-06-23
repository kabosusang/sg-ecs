#pragma once
#include <cstddef>
#include <flat_set>

#include "core/component_mask.hpp"
#include "table.hpp"

namespace ecs {

struct archetype {
    explicit archetype(size_t id = 0) :id_(id){}
    
	size_t id_ = SIZE_MAX;
	component_mask table_mask_;
	std::flat_set<component_id_type> table_comps_;
	table table_;

	[[nodiscard]] bool has(component_id_type id) const noexcept {
		return table_mask_.test(id);
	}
};

class archetype_graph {
	std::flat_map<uint64_t, size_t> edges_;
	[[nodiscard]] static constexpr uint64_t encode(size_t from_archetype_id, component_id_type component_id) noexcept {
		return (from_archetype_id << 32) | (component_id & 0xFFFFFFFFull);
	}

public:
	[[nodiscard]] size_t find(size_t from, component_id_type component_id) const noexcept {
		auto it = edges_.find(encode(from, component_id));
		return it != edges_.end() ? it->second : SIZE_MAX;
	}

	void insert(size_t from_archetype_id, component_id_type component_id, size_t to_arcgetype_id) noexcept {
		edges_[encode(from_archetype_id, component_id)] = to_arcgetype_id;
	}
	[[nodiscard]] size_t edge_count() const noexcept { return edges_.size(); }
};

} //namespace ecs
