#pragma once

#include <cassert>
#include <flat_map>
#include <functional>
#include <string>

#include <concepts>
#include <functional>
#include <span>

#include "component.hpp"
#include "component_info.hpp"
#include "storage/blob_array.hpp"

namespace ecs {

class component_registry {
public:
	template <typename T>
	component_id_type register_component(std::string name, storage_type st) {
		component_id_type cid = type_index<T>();
		infos_.try_emplace(cid,
				std::move(name), // name_
				st, // storage_type_
				sizeof(T), // size_bytes_
				alignof(T), // align_bytes_
				&blob_array_accessor<T>::make_blob_array // make_blob_array_
		);
		return cid;
	}

	[[nodiscard]] const component_info* get(component_id_type cid) const noexcept {
		auto it = infos_.find(cid);
		return it != infos_.end() ? &it->second : nullptr;
	}

	blob_array make_column(component_id_type cid) const {
		auto* info = get(cid);
		assert(info && "Component not registered");
		return info->make_blob_array_();
	}

	[[nodiscard]] bool is_table(component_id_type cid) const noexcept {
		auto* p = get(cid);
		return p && p->is_table();
	}

	[[nodiscard]] bool is_sparse_set(component_id_type cid) const noexcept {
		auto* p = get(cid);
		return p && p->is_sparse_set();
	}

	[[nodiscard]] const std::string& name(component_id_type cid) const noexcept {
		static std::string unknown = "<?>";
		auto* p = get(cid);
		return p ? p->name_ : unknown;
	}

private:
	std::flat_map<component_id_type, component_info> infos_;
};

} //namespace ecs
