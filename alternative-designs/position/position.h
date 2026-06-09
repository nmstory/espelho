#pragma once

#include <cstdint>

enum class TypeID : uint16_t {
	Position = 1,
};

struct VisitorPosition {
	float x = 0, y = 0;
};
