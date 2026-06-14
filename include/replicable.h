#pragma once

#include <cstdint>
#include <stream.h>

/* 	TODO: investigate moving IDs into each type (static constexpr TypeID ms_TypeID)
	to avoid recompiling all types when this enum grows. CMakeable?
	IDs are wire protocol — existing values must never change or be reused. */
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
