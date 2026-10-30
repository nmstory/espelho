#pragma once

#include <filesystem>
#include <vector>

// The world is a grid of objects described by a shared text file, one object
// per line: "id x y owner". An owner of 0 means the object is free; otherwise
// it is the port of the peer that has claimed it.
constexpr int worldCols = 40;
constexpr int worldRows = 25;
constexpr int worldSpacing = 10;
constexpr int worldObjectCount = worldCols * worldRows;

struct WorldEntry
{
  int id = 0;
  int x = 0;
  int y = 0;
  int owner = 0;
};

[[nodiscard]] std::vector<WorldEntry> LoadWorld(
    const std::filesystem::path& path);
void SaveWorld(const std::filesystem::path& path,
               const std::vector<WorldEntry>& entries);

// Writes the deterministic starting world: every object free, on a grid.
void SeedWorld(const std::filesystem::path& path);

// Cross-process lock held for a read-modify-write of the world file. Relies on
// directory creation being atomic; blocks until the lock is acquired. A lock
// left behind by a crashed process must be removed by hand.
class WorldLock
{
public:
  explicit WorldLock(std::filesystem::path path);
  ~WorldLock();

  WorldLock(const WorldLock&) = delete;
  WorldLock& operator=(const WorldLock&) = delete;

private:
  std::filesystem::path path;
};

[[nodiscard]] bool InRadius(int x, int y, int cx, int cy, int radius);

// Marks every free entry within `radius` of (cx, cy) as owned by `port`, and
// returns copies of the entries claimed.
[[nodiscard]] std::vector<WorldEntry> ClaimNearby(
    std::vector<WorldEntry>& entries, int cx, int cy, int radius, int port);

// Writes (x, y) back to the entry `id` and frees it. Only succeeds if the
// entry is currently owned by `port`.
bool ReleaseEntry(
    std::vector<WorldEntry>& entries, int id, int x, int y, int port);
