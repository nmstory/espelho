#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <numbers>
#include <thread>
#include <vector>

#include <espelho.h>
#include <position.h>
#include <world_file.h>

using namespace std::chrono_literals;

namespace
{

const std::filesystem::path worldPath = "world.txt";
const std::filesystem::path lockPath = "world.txt.lock";

// Each peer's viewpoint circles the centre of the world; a peer claims every
// free object within viewRadius of it.
constexpr int viewRadius = 60;
constexpr int pathRadius = 100;
constexpr int worldCentreX = (worldCols - 1) * worldSpacing / 2;
constexpr int worldCentreY = (worldRows - 1) * worldSpacing / 2;
constexpr double angularSpeed = 0.05;  // radians per tick

struct Viewpoint
{
  int x;
  int y;
};

// The two peers start on opposite sides and circle in opposite directions, so
// their bubbles meet part-way round.
Viewpoint viewpointAt(int tick, bool lowerPort)
{
  const double start = lowerPort ? 0.0 : std::numbers::pi;
  const double direction = lowerPort ? 1.0 : -1.0;
  const double angle = start + direction * angularSpeed * tick;
  return {worldCentreX + static_cast<int>(pathRadius * std::cos(angle)),
          worldCentreY + static_cast<int>(pathRadius * std::sin(angle))};
}

// Everything in `owned` is a Position, so the downcast is safe.
Position& asPosition(const std::unique_ptr<Replicable>& obj)
{
  return static_cast<Position&>(*obj);
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

  {
    WorldLock lock(lockPath);
    if (!std::filesystem::exists(worldPath)) {
      SeedWorld(worldPath);
    }
  }

  std::vector<std::unique_ptr<Replicable>> owned;

  for (int tick = 0;; ++tick) {
    const Viewpoint view = viewpointAt(tick, ownPort < peerPort);

    // owned objects drift, so a release has a changed position to write back
    for (const auto& obj : owned) {
      Position& p = asPosition(obj);
      p.set(p.x() + 1, p.y());
    }

    {
      WorldLock lock(lockPath);
      std::vector<WorldEntry> entries = LoadWorld(worldPath);

      std::erase_if(owned,
                    [&](const std::unique_ptr<Replicable>& obj)
                    {
                      const Position& p = asPosition(obj);
                      if (InRadius(p.x(), p.y(), view.x, view.y, viewRadius)) {
                        return false;
                      }
                      (void)ReleaseEntry(entries, p.id, p.x(), p.y(), ownPort);
                      return true;
                    });

      for (const WorldEntry& e :
           ClaimNearby(entries, view.x, view.y, viewRadius, ownPort))
      {
        auto position = std::make_unique<Position>(e.x, e.y);
        position->id = e.id;
        owned.push_back(std::move(position));
      }

      SaveWorld(worldPath, entries);
    }

    espelho.SendObjects(owned);
    espelho.Update();

    std::cout << "tick " << tick << " | view (" << view.x << ", " << view.y
              << ") | owned " << owned.size() << " | mirror "
              << espelho.Objects().size() << std::endl;

    std::this_thread::sleep_for(100ms);
  }
}
