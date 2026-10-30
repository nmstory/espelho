#include <filesystem>

#include <gtest/gtest.h>
#include <world_file.h>

namespace
{

std::filesystem::path tempPath(const char* name)
{
  return std::filesystem::temp_directory_path() / name;
}

}  // namespace

TEST(WorldFile, SeedThenLoadProducesFreeGrid)
{
  const auto path = tempPath("espelho_world_seed_test.txt");
  SeedWorld(path);
  const auto entries = LoadWorld(path);
  std::filesystem::remove(path);

  ASSERT_EQ(static_cast<size_t>(worldObjectCount), entries.size());
  EXPECT_EQ(1, entries.front().id);
  EXPECT_EQ(worldObjectCount, entries.back().id);
  for (const auto& e : entries) {
    EXPECT_EQ(0, e.owner);
  }
}

TEST(WorldFile, SaveLoadRoundtrip)
{
  const auto path = tempPath("espelho_world_roundtrip_test.txt");
  const std::vector<WorldEntry> original = {{1, 5, -7, 0}, {2, 30, 40, 7000}};
  SaveWorld(path, original);
  const auto loaded = LoadWorld(path);
  std::filesystem::remove(path);

  ASSERT_EQ(original.size(), loaded.size());
  for (size_t i = 0; i < original.size(); ++i) {
    EXPECT_EQ(original[i].id, loaded[i].id);
    EXPECT_EQ(original[i].x, loaded[i].x);
    EXPECT_EQ(original[i].y, loaded[i].y);
    EXPECT_EQ(original[i].owner, loaded[i].owner);
  }
}

TEST(WorldClaim, TakesOnlyFreeEntriesInRadius)
{
  std::vector<WorldEntry> entries = {
      {1, 0, 0, 0},      // free, in range
      {2, 100, 0, 0},    // free, out of range
      {3, 5, 5, 0},      // free, in range
  };
  const auto claimed = ClaimNearby(entries, 0, 0, 20, 7000);

  ASSERT_EQ(2u, claimed.size());
  EXPECT_EQ(7000, entries[0].owner);
  EXPECT_EQ(0, entries[1].owner);
  EXPECT_EQ(7000, entries[2].owner);
}

TEST(WorldClaim, SkipsEntriesOwnedByAnotherPeer)
{
  std::vector<WorldEntry> entries = {{1, 0, 0, 7001}};
  const auto claimed = ClaimNearby(entries, 0, 0, 20, 7000);

  EXPECT_TRUE(claimed.empty());
  EXPECT_EQ(7001, entries[0].owner);
}

TEST(WorldClaim, RadiusIsInclusive)
{
  std::vector<WorldEntry> entries = {{1, 20, 0, 0}};
  EXPECT_EQ(1u, ClaimNearby(entries, 0, 0, 20, 7000).size());
}

TEST(WorldRelease, WritesPositionBackAndFreesEntry)
{
  std::vector<WorldEntry> entries = {{1, 0, 0, 7000}};
  EXPECT_TRUE(ReleaseEntry(entries, 1, 42, 17, 7000));

  EXPECT_EQ(42, entries[0].x);
  EXPECT_EQ(17, entries[0].y);
  EXPECT_EQ(0, entries[0].owner);
}

TEST(WorldRelease, RefusesEntryOwnedByAnotherPeer)
{
  std::vector<WorldEntry> entries = {{1, 0, 0, 7001}};
  EXPECT_FALSE(ReleaseEntry(entries, 1, 42, 17, 7000));

  EXPECT_EQ(0, entries[0].x);
  EXPECT_EQ(7001, entries[0].owner);
}

TEST(WorldLockTest, ReleasesOnDestruction)
{
  const auto path = tempPath("espelho_world_lock_test");
  {
    WorldLock lock(path);
    EXPECT_TRUE(std::filesystem::exists(path));
  }
  EXPECT_FALSE(std::filesystem::exists(path));
}
