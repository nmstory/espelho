#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <espelho.h>
#include <health.h>
#include <position.h>

using namespace std::chrono_literals;

namespace
{

void printMirror(const std::vector<std::unique_ptr<Replicable>>& objects)
{
  std::cout << " | mirror:";
  if (objects.empty()) {
    std::cout << " (nothing yet)";
  }
  for (const auto& obj : objects) {
    switch (obj->typeID()) {
      case TypeID::Position: {
        const auto& position = static_cast<const Position&>(*obj);
        std::cout << " Position#" << obj->id << "(" << position.x() << ", "
                  << position.y() << ")";
        break;
      }
      case TypeID::Health: {
        const auto& health = static_cast<const Health&>(*obj);
        std::cout << " Health#" << obj->id << "("
                  << static_cast<int>(health.value()) << ")";
        break;
      }
    }
  }
  std::cout << std::endl;
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

  auto ownedPosition = std::make_unique<Position>(0, 0);
  Position* position = ownedPosition.get();

  std::vector<std::unique_ptr<Replicable>> objects;
  objects.push_back(std::move(ownedPosition));
  objects.push_back(std::make_unique<Health>(100));
  for (auto& obj : objects) {
    obj->id = ownPort;  // tag records with their origin
  }

  for (int tick = 0;; ++tick) {
    position->set(tick, 0);

    espelho.SendObjects(objects);
    espelho.Update();

    std::cout << "tick " << tick << " | sent Position#" << ownPort << "("
              << tick << ", 0)";
    printMirror(espelho.Objects());

    std::this_thread::sleep_for(100ms);
  }
}
