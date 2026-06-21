#pragma once
#include <cstddef>
#include <string>

#include "storage/storage_type.hpp"

namespace ecs {

struct component_info {
	std::string name_;
	storage_type storage_type_ = storage_type::table;
	size_t size_bytes_ = 0;
	size_t align_bytes_ = 0;
	using blob_array_factory = class blob_array (*)();
	blob_array_factory make_blob_array_ = nullptr;

	[[nodiscard]] bool is_table() const noexcept { return storage_type_ == storage_type::table; }
	[[nodiscard]] bool is_sparse_set() const noexcept { return storage_type_ == storage_type::sparse_set; }
};

} //namespace ecs
