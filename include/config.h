#pragma once

#include <cstddef>

namespace espelho::config
{

/*  Conservative MTU budget in bytes. Stays below IPv6's 1280 minimum MTU
    after IP + UDP headers, avoiding fragmentation on any network path. */
inline constexpr size_t MTU_BYTES = 1200;

}  // namespace espelho::config
