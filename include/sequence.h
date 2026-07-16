#pragma once

#include <cstdint>

/* Wraparound-aware comparison over the circular uint16_t space: a is "greater"
   than b when it is at most half the sequence space ahead, so 0 beats 65535. */
constexpr bool sequence_greater_than(uint16_t a, uint16_t b)
{
  return ((a > b) && (a - b <= 32768)) || ((a < b) && (b - a > 32768));
}
