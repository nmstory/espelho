#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <espelho.h>
#include <health.h>
#include <position.h>

using namespace std::chrono_literals;

namespace
{

// Enough entities to force packet-splitting under the 1200-byte MTU budget,
// as a stand-in for a real game world ahead of phase two's optimisation work.
constexpr int objectCount = 1000;

void printMirror(const std::vector<std::unique_ptr<Replicable>>& objects)
{
  size_t positions = 0;
  size_t healths = 0;
  for (const auto& obj : objects) {
    switch (obj->typeID()) {
      case TypeID::Position:
        ++positions;
        break;
      case TypeID::Health:
        ++healths;
        break;
    }
  }
  std::cout << " | mirror: " << positions << " Position, " << healths
            << " Health";
}

}  // namespace

int main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <own-port> <peer-port>" << std::endl;
    return 1;
  }

  const int ownPort = std::stoi(argv[1]);
  const int peerPort = std::stoi(argv[2]);

  Espelho espelho(ownPort, 10ms);
  if (!espelho.AddPeer("127.0.0.1", peerPort)) {
    std::cerr << "Error: failed to add peer on port " << peerPort << std::endl;
    return 1;
  }

  std::vector<std::unique_ptr<Replicable>> objects;
  objects.reserve(objectCount * 2);
  std::vector<Position*> positions;
  positions.reserve(objectCount);
  for (int i = 0; i < objectCount; ++i) {
    const int id = ownPort * 10000 + i;  // unique per entity, tagged by origin

    auto position = std::make_unique<Position>(i, 0);
    position->id = id;
    positions.push_back(position.get());
    objects.push_back(std::move(position));

    auto health = std::make_unique<Health>();
    health->id = id;
    objects.push_back(std::move(health));
  }

  for (int tick = 0;; ++tick) {
    for (int i = 0; i < objectCount; ++i) {
      positions[i]->set(tick + i, i);
    }

    espelho.SendObjects(objects);
    espelho.Update();

    std::cout << "tick " << tick << " | sent " << objectCount << " objects";
    printMirror(espelho.Objects());
    std::cout << std::endl;

    std::this_thread::sleep_for(100ms);
  }
}
