#include <chrono>
#include <cstring>
#include <iostream>
#include <client.h>
#include <id.h>
#include "position.h"

struct PacketVisitor {
	virtual void visit(ID, VisitorPosition&) = 0;
	virtual ~PacketVisitor() = default;
};

struct PrintVisitor : PacketVisitor {
	void visit(ID id, VisitorPosition& p) override {
		std::cout << "client " << getClientID(id)
		          << " object " << getObjectID(id)
		          << " position: (" << p.x << ", " << p.y << ")\n";
	}
};

void deserialise(const std::vector<uint8_t>& bytes, PacketVisitor& v) {
	constexpr size_t header = sizeof(TypeID) + sizeof(ID);
	if (bytes.size() < header) return;

	TypeID tid;
	std::memcpy(&tid, bytes.data(), sizeof(tid));

	ID id;
	std::memcpy(&id, bytes.data() + sizeof(tid), sizeof(id));

	switch (tid) {
		case TypeID::Position: {
			VisitorPosition p;
			std::memcpy(&p, bytes.data() + header, sizeof(p));
			v.visit(id, p);
			break;
		}
	}
}

int main() {
	Client receiver;
	receiver.init("127.0.0.1", 5002, std::chrono::milliseconds(500));

	Client sender;
	sender.init("127.0.0.1", 5001);
	sender.addPeer("127.0.0.1", 5002);

	TypeID tid = TypeID::Position;
	ID id = makeID(7, 1);
	VisitorPosition pos{1.0f, 2.0f};

	uint8_t buf[sizeof(tid) + sizeof(id) + sizeof(pos)];
	std::memcpy(buf, &tid, sizeof(tid));
	std::memcpy(buf + sizeof(tid), &id, sizeof(id));
	std::memcpy(buf + sizeof(tid) + sizeof(id), &pos, sizeof(pos));
	sender.send(buf, sizeof(buf));

	PrintVisitor visitor;
	for (int i = 0; i < 5; ++i) {
		auto bytes = receiver.update();
		if (bytes) {
			deserialise(*bytes, visitor);
			return 0;
		}
	}

	std::cerr << "no data received\n";
	return 1;
}
