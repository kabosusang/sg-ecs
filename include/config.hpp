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

//component id
#ifndef COMPONENT_ID_TYPE
  #include <cstdint>
  #define COMPONENT_ID_TYPE std::uint32_t
#else
#endif



