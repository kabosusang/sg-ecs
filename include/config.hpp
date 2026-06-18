#pragma once

// entity id
#ifndef ENTITY_ID_TYPE
#include <cstdint>
#define ENTITY_ID_TYPE std::uint64_t
#else
#endif
// entity [index,version]
#ifndef ENTITY_INDEX_BITS
#define ENTITY_INDEX_BITS 48
#endif

