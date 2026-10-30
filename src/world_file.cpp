#include <chrono>
#include <fstream>
#include <stdexcept>
#include <system_error>
#include <thread>

#include <world_file.h>

using namespace std::chrono_literals;

std::vector<WorldEntry> LoadWorld(const std::filesystem::path& path)
{
  std::vector<WorldEntry> entries;
  std::ifstream in(path);
  WorldEntry e;
  while (in >> e.id >> e.x >> e.y >> e.owner) {
    entries.push_back(e);
  }
  return entries;
}

void SaveWorld(const std::filesystem::path& path,
               const std::vector<WorldEntry>& entries)
{
  std::ofstream out(path, std::ios::trunc);
  for (const WorldEntry& e : entries) {
    out << e.id << ' ' << e.x << ' ' << e.y << ' ' << e.owner << '\n';
  }
}

void SeedWorld(const std::filesystem::path& path)
{
  std::vector<WorldEntry> entries;
  entries.reserve(worldObjectCount);
  for (int row = 0; row < worldRows; ++row) {
    for (int col = 0; col < worldCols; ++col) {
      const int id = row * worldCols + col + 1;
      entries.push_back({id, col * worldSpacing, row * worldSpacing, 0});
    }
  }
  SaveWorld(path, entries);
}

WorldLock::WorldLock(std::filesystem::path lockPath)
    : path(std::move(lockPath))
{
  std::error_code ec;
  // false with no error means another process holds the lock
  while (!std::filesystem::create_directory(path, ec)) {
    if (ec) {
      throw std::runtime_error("WorldLock: " + ec.message());
    }
    std::this_thread::sleep_for(1ms);
  }
}

WorldLock::~WorldLock()
{
  std::error_code ec;
  std::filesystem::remove(path, ec);
}

bool InRadius(int x, int y, int cx, int cy, int radius)
{
  const long long dx = x - cx;
  const long long dy = y - cy;
  return dx * dx + dy * dy <= static_cast<long long>(radius) * radius;
}

std::vector<WorldEntry> ClaimNearby(
    std::vector<WorldEntry>& entries, int cx, int cy, int radius, int port)
{
  std::vector<WorldEntry> claimed;
  for (WorldEntry& e : entries) {
    if (e.owner == 0 && InRadius(e.x, e.y, cx, cy, radius)) {
      e.owner = port;
      claimed.push_back(e);
    }
  }
  return claimed;
}

bool ReleaseEntry(
    std::vector<WorldEntry>& entries, int id, int x, int y, int port)
{
  for (WorldEntry& e : entries) {
    if (e.id == id) {
      if (e.owner != port) {
        return false;
      }
      e.x = x;
      e.y = y;
      e.owner = 0;
      return true;
    }
  }
  return false;
}
