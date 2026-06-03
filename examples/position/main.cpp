#include <cstdint>
#include <iostream>
#include <id.h>
#include <stream.h>
#include "position.h"

int main() {
	PositionReplicable sender;
	sender.id = makeID(7, 1);
	sender.position = {1.0f, 2.0f};

	uint8_t buf[64];
	WriteStream ws{buf, sizeof(buf)};
	if (!sender.process(ws)) {
		std::cerr << "write failed\n";
		return 1;
	}

	PositionReplicable mirror;
	ReadStream rs{buf, ws.pos};
	if (!mirror.process(rs)) {
		std::cerr << "read failed\n";
		return 1;
	}

	if (mirror.position.x != sender.position.x ||
	    mirror.position.y != sender.position.y) {
		std::cerr << "round-trip mismatch: ("
		          << mirror.position.x << ", " << mirror.position.y << ")\n";
		return 1;
	}

	std::cout << "client " << getClientID(sender.id)
	          << " object " << getObjectID(sender.id)
	          << " pos: (" << mirror.position.x << ", " << mirror.position.y << ")\n";
	return 0;
}
