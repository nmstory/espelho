#pragma once

#include <stream.h>

class Replicable {
public:
	virtual bool process(WriteStream& s) = 0;
	virtual bool process(ReadStream& s)  = 0;
	virtual ~Replicable() = default;

	int id = 0;
};
