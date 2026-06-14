#pragma once

#include <cstdint>
#include <stream.h>

enum class TypeID : uint16_t {
	Position = 1,
};

class Replicable {
public:
	virtual ~Replicable() = default;

	virtual bool process(WriteStream& s) = 0;
	virtual bool process(ReadStream& s)  = 0;
	virtual TypeID typeID() const = 0;

	int id = 0;
};
