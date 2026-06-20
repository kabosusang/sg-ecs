#include <cstddef>
#pragma once
#include <cstdint>
#include <vector>

#include "entity.hpp"
#include "entity_location.hpp"

namespace ecs {

struct entity_index {
	[[nodiscard]] entity spawn() {
		uint64_t idx;
		if (!free_list_.empty()) {
			idx = free_list_.back();
			free_list_.pop_back();
		} else {
			idx = next_entity_++;
		}
		ensure_slot(idx);
		return entity(idx, versions_[idx]);
	}

	void despawn(entity e) noexcept {
		uint64_t idx = e.index();
		if (idx >= locations_.size()) {
			return;
		}
		if (versions_[idx] != e.version()) {
			return; // 版本不匹配 = 已死/过期
		}
		versions_[idx]++; // bump → 旧引用失效
		locations_[idx] = { INVALID_ROW, INVALID_ROW };
		free_list_.push_back(idx);
	}

	[[nodiscard]] bool is_alive(entity e) const noexcept {
		uint64_t idx = e.index();
		return idx < locations_.size() && versions_[idx] == e.version();
	}

	// 位置访问 (调用方负责先做 is_alive)
	[[nodiscard]] entity_location& location(entity e) noexcept { return locations_[e.index()]; }
	[[nodiscard]] const entity_location& location(entity e) const noexcept { return locations_[e.index()]; }

	[[nodiscard]] size_t entity_count() const noexcept {
		return next_entity_ - free_list_.size();
	}
	[[nodiscard]] size_t capacity() const noexcept { return locations_.size(); }

private:
	void ensure_slot(uint64_t idx) {
		if (idx >= locations_.size()) {
			locations_.resize(idx + 1);
			versions_.resize(idx + 1, 0);
		}
	}

	std::vector<entity_location> locations_; // 每个index对应的archetype和行号
	std::vector<uint64_t> versions_; // 每个 index 的当前 version
	std::vector<uint64_t> free_list_; // 回收的 index
	uint64_t next_entity_ = 0;
};

} //namespace ecs
