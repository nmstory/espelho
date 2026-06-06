#pragma once

#include <cstdint>

using ID = uint32_t;

constexpr uint32_t CLIENT_BITS = 8;
constexpr uint32_t LOCAL_BITS  = 32 - CLIENT_BITS;

constexpr uint32_t LOCAL_MASK  = (1u << LOCAL_BITS) - 1;
constexpr uint32_t CLIENT_MASK = (1u << CLIENT_BITS) - 1;

inline ID makeID(uint32_t clientID, uint32_t localID) {
	return ((clientID & CLIENT_MASK) << LOCAL_BITS) |
	       (localID  & LOCAL_MASK);
}

inline uint32_t getClientID(ID id) {
	return (id >> LOCAL_BITS) & CLIENT_MASK;
}

inline uint32_t getObjectID(ID id) {
	return id & LOCAL_MASK;
}
