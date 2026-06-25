#pragma once
#include <atomic>
#include <string>
#include <string_view>

#include "config.hpp"

#if defined(__clang__) || defined(__GNUC__)
#define ECS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define ECS_PRETTY_FUNCTION_PREFIX '='
#define ECS_PRETTY_FUNCTION_SUFFIX ']'
#elif defined(_MSC_VER)
#define ECS_PRETTY_FUNCTION __FUNCSIG__
#define ECS_PRETTY_FUNCTION_PREFIX '<'
#define ECS_PRETTY_FUNCTION_SUFFIX '>'
#endif

namespace ecs {
using component_id_type = COMPONENT_ID_TYPE;

inline component_id_type next_type_index() noexcept {
	static std::atomic<component_id_type> counter{ 0 };
	return counter.fetch_add(1, std::memory_order_relaxed);
}
//component
template <typename T>
[[nodiscard]] component_id_type type_index() noexcept {
	static const component_id_type id = next_type_index();
	return id;
}

// hash_type
struct hashed_string {
	//hash to string
	using hash_type = component_id_type;
	static constexpr hash_type offset = []() -> hash_type {
		if constexpr (sizeof(hash_type) == 4) {
			return 2166136261u;
		} else if constexpr (sizeof(hash_type) == 8) {
			return 14695981039346656037ull;
		}
	}();

	static constexpr hash_type prime = []() -> hash_type {
		if constexpr (sizeof(hash_type) == 4) {
			return 16777619u;
		} else if constexpr (sizeof(hash_type) == 8) {
			return 1099511628211ull;
		}
	}();

	static constexpr hash_type value(const char* str, size_t len) {
		hash_type hash = offset;
		for (size_t i = 0; i < len; ++i) {
			hash ^= static_cast<hash_type>(str[i]);
			hash *= prime;
		}
		return hash;
	}
};

template <typename T>
[[nodiscard]] inline constexpr auto pretty_function() {
	return ECS_PRETTY_FUNCTION;
}

template <typename T>
[[nodiscard]] inline constexpr auto stripped_type_name() {
	std::string_view full{ pretty_function<T>() };
	auto start = full.find_first_of(ECS_PRETTY_FUNCTION_PREFIX) + 1;
	auto end = full.find_last_of(ECS_PRETTY_FUNCTION_SUFFIX);
	auto name = full.substr(start, end - start);

	// 1. 去除前导和尾随空格
	auto first = name.find_first_not_of(' ');
	if (first == std::string_view::npos) {
		return std::string_view{};
	}
	auto last = name.find_last_not_of(' ');
	name = name.substr(first, last - first + 1);

	// 2. 去除 "struct " 或 "class " 前缀
	if (name.starts_with("struct ")) {
		name.remove_prefix(7);
	} else if (name.starts_with("class ")) {
		name.remove_prefix(6);
	} else if (name.starts_with("enum ")) {
		name.remove_prefix(5);
	}

	return name;
}

//编译期哈希
template <typename T>
[[nodiscard]] constexpr component_id_type type_hash() {
	constexpr auto name = stripped_type_name<T>();
	return hashed_string::value(name.data(), name.size());
}

//运行时哈希（
inline component_id_type type_hash(const std::string& name) {
	return hashed_string::value(name.data(), name.size());
}



// ==========================================
//resource index
inline component_id_type next_resource_id() noexcept {
    static std::atomic<component_id_type> counter{ 0 };
    return counter.fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
component_id_type resource_type_id() noexcept {
    static const component_id_type id = next_resource_id();
    return id;
}





















} //namespace ecs
