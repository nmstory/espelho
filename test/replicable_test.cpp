#include <cstring>

#include <gtest/gtest.h>
#include <health.h>
#include <position.h>

TEST(HealthRoundtrip, WriteThenReadProducesSameBytes)
{
  uint8_t writeBuf[16] = {};
  Health original {42};
  WriteStream ws {writeBuf, sizeof(writeBuf)};
  ASSERT_TRUE(original.process(ws));

  ReadStream rs {writeBuf, ws.pos};
  Health roundtripped;
  ASSERT_TRUE(roundtripped.process(rs));

  uint8_t rewriteBuf[16] = {};
  WriteStream rws {rewriteBuf, sizeof(rewriteBuf)};
  ASSERT_TRUE(roundtripped.process(rws));

  EXPECT_EQ(ws.pos, rws.pos);
  EXPECT_EQ(0, std::memcmp(writeBuf, rewriteBuf, ws.pos));
}

TEST(HealthTypeID, IsHealth)
{
  Health health;
  EXPECT_EQ(TypeID::Health, health.typeID());
}

TEST(HealthFactory, MakeReturnsHealth)
{
  auto replicable = Health::make();
  ASSERT_NE(nullptr, replicable);
  EXPECT_EQ(TypeID::Health, replicable->typeID());
}

TEST(HealthStream, RejectsBufferTooSmall)
{
  uint8_t tinyBuf[1];
  ReadStream rs {tinyBuf, 0};
  Health health;
  EXPECT_FALSE(health.process(rs));
}

TEST(PositionRoundtrip, WriteThenReadProducesSameBytes)
{
  uint8_t writeBuf[16] = {};
  Position original {3, -7};
  WriteStream ws {writeBuf, sizeof(writeBuf)};
  ASSERT_TRUE(original.process(ws));

  ReadStream rs {writeBuf, ws.pos};
  Position roundtripped;
  ASSERT_TRUE(roundtripped.process(rs));

  uint8_t rewriteBuf[16] = {};
  WriteStream rws {rewriteBuf, sizeof(rewriteBuf)};
  ASSERT_TRUE(roundtripped.process(rws));

  EXPECT_EQ(ws.pos, rws.pos);
  EXPECT_EQ(0, std::memcmp(writeBuf, rewriteBuf, ws.pos));
}

TEST(PositionTypeID, IsPosition)
{
  Position position;
  EXPECT_EQ(TypeID::Position, position.typeID());
}

TEST(PositionFactory, MakeReturnsPosition)
{
  auto replicable = Position::make();
  ASSERT_NE(nullptr, replicable);
  EXPECT_EQ(TypeID::Position, replicable->typeID());
}

TEST(PositionStream, RejectsBufferTooSmall)
{
  uint8_t tinyBuf[4];
  ReadStream rs {tinyBuf, sizeof(tinyBuf)};
  Position position;
  EXPECT_FALSE(position.process(rs));
}
