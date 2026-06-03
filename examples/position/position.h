#pragma once

#include <replicable.h>
#include <stream.h>

struct Position { float x = 0, y = 0; };

struct PositionReplicable : public Replicable {
	Position position;

	bool process(WriteStream& s) override { return process_impl(s); }
	bool process(ReadStream& s)  override { return process_impl(s); }

private:
	template<typename Stream>
	bool process_impl(Stream& s) {
		return process_pod(s, position.x) && process_pod(s, position.y);
	}
};
