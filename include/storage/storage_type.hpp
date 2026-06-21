#pragma once
#include <cstdint>


namespace ecs {
enum class storage_type : uint8_t {
    table,     //archetype
    sparse_set,//sparse_set
};
}


