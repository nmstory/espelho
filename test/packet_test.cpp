#include <cstring>
#include <vector>

#include <gtest/gtest.h>
#include <health.h>
#include <packet_reader.h>
#include <packet_writer.h>
#include <position.h>
#include <sequence.h>

namespace
{

// hand-craft a single-record packet: seq header, then (TypeID, id, x, y)
std::vector<uint8_t> makePositionPacket(uint16_t seq, int id, int x, int y)
{
  std::vector<uint8_t> buf(espelho::config::MTU_BYTES);
  WriteStream stream {buf.data(), buf.size()};
  TypeID tid = TypeID::Position;
  EXPECT_TRUE(process_pod(stream, seq) && process_pod(stream, tid)
              && process_pod(stream, id) && process_pod(stream, x)
              && process_pod(stream, y));
  buf.resize(stream.pos);
  return buf;
}

TypeRegistry makeRegistry()
{
  TypeRegistry registry;
  registry.add(TypeID::Position, &Position::make);
  registry.add(TypeID::Health, &Health::make);
  return registry;
}

const Position& positionAt(
    const std::vector<std::unique_ptr<Replicable>>& objects, size_t i)
{
  return static_cast<const Position&>(*objects[i]);
}

}  // namespace

TEST(Sequence, GreaterThan)
{
  EXPECT_TRUE(sequence_greater_than(1, 0));
  EXPECT_FALSE(sequence_greater_than(0, 1));
  EXPECT_FALSE(sequence_greater_than(0, 0));
  EXPECT_TRUE(sequence_greater_than(0, 65535));  // wraparound
  EXPECT_FALSE(sequence_greater_than(65535, 0));
  EXPECT_TRUE(sequence_greater_than(32768, 0));  // half-range edge
  EXPECT_FALSE(sequence_greater_than(32769, 0));
}

TEST(PacketWriterSeq, EmitsIncrementingSeq)
{
  PacketWriter writer;
  Position position {1, 2};

  ASSERT_TRUE(writer.write(position));
  uint16_t seq = 0xffff;
  std::memcpy(&seq, writer.data(), sizeof(seq));
  EXPECT_EQ(0, seq);

  writer.reset();
  ASSERT_TRUE(writer.write(position));
  std::memcpy(&seq, writer.data(), sizeof(seq));
  EXPECT_EQ(1, seq);
}

TEST(PacketWriterSeq, HasRecordsOnlyAfterWrite)
{
  PacketWriter writer;
  EXPECT_FALSE(writer.hasRecords());

  Position position;
  ASSERT_TRUE(writer.write(position));
  EXPECT_TRUE(writer.hasRecords());

  writer.reset();
  EXPECT_FALSE(writer.hasRecords());
}

TEST(PacketReaderSeq, FirstPacketAccepted)
{
  TypeRegistry registry = makeRegistry();
  std::vector<std::unique_ptr<Replicable>> objects;
  PacketReader reader;

  auto packet = makePositionPacket(7, 1, 10, 20);
  reader.read(packet.data(), packet.size(), registry, objects);

  ASSERT_EQ(1u, objects.size());
  EXPECT_EQ(1, objects[0]->id);
  EXPECT_EQ(10, positionAt(objects, 0).x());
  EXPECT_EQ(20, positionAt(objects, 0).y());
}

TEST(PacketReaderSeq, NewerPacketAccepted)
{
  TypeRegistry registry = makeRegistry();
  std::vector<std::unique_ptr<Replicable>> objects;
  PacketReader reader;

  auto first = makePositionPacket(0, 1, 5, 5);
  reader.read(first.data(), first.size(), registry, objects);
  auto second = makePositionPacket(1, 1, 6, 7);
  reader.read(second.data(), second.size(), registry, objects);

  ASSERT_EQ(1u, objects.size());
  EXPECT_EQ(6, positionAt(objects, 0).x());
  EXPECT_EQ(7, positionAt(objects, 0).y());
}

TEST(PacketReaderSeq, StalePacketDropped)
{
  TypeRegistry registry = makeRegistry();
  std::vector<std::unique_ptr<Replicable>> objects;
  PacketReader reader;

  auto newer = makePositionPacket(5, 1, 1, 1);
  reader.read(newer.data(), newer.size(), registry, objects);
  auto stale = makePositionPacket(3, 1, 9, 9);
  reader.read(stale.data(), stale.size(), registry, objects);

  ASSERT_EQ(1u, objects.size());
  EXPECT_EQ(1, positionAt(objects, 0).x());
  EXPECT_EQ(1, positionAt(objects, 0).y());
}

TEST(PacketReaderSeq, DuplicatePacketDropped)
{
  TypeRegistry registry = makeRegistry();
  std::vector<std::unique_ptr<Replicable>> objects;
  PacketReader reader;

  auto first = makePositionPacket(4, 1, 1, 1);
  reader.read(first.data(), first.size(), registry, objects);
  auto duplicate = makePositionPacket(4, 1, 9, 9);
  reader.read(duplicate.data(), duplicate.size(), registry, objects);

  ASSERT_EQ(1u, objects.size());
  EXPECT_EQ(1, positionAt(objects, 0).x());
}

TEST(PacketReaderSeq, WraparoundAccepted)
{
  TypeRegistry registry = makeRegistry();
  std::vector<std::unique_ptr<Replicable>> objects;
  PacketReader reader;

  auto beforeWrap = makePositionPacket(65535, 1, 1, 1);
  reader.read(beforeWrap.data(), beforeWrap.size(), registry, objects);
  auto afterWrap = makePositionPacket(0, 1, 2, 2);
  reader.read(afterWrap.data(), afterWrap.size(), registry, objects);

  ASSERT_EQ(1u, objects.size());
  EXPECT_EQ(2, positionAt(objects, 0).x());
  EXPECT_EQ(2, positionAt(objects, 0).y());
}

TEST(PacketRoundtrip, WriterToReader)
{
  PacketWriter writer;
  Position position {3, 4};
  position.id = 11;
  Health health {42};
  health.id = 12;
  ASSERT_TRUE(writer.write(position));
  ASSERT_TRUE(writer.write(health));

  TypeRegistry registry = makeRegistry();
  std::vector<std::unique_ptr<Replicable>> objects;
  PacketReader reader;
  reader.read(writer.data(), writer.size(), registry, objects);

  ASSERT_EQ(2u, objects.size());
  EXPECT_EQ(11, objects[0]->id);
  EXPECT_EQ(3, positionAt(objects, 0).x());
  EXPECT_EQ(4, positionAt(objects, 0).y());
  EXPECT_EQ(12, objects[1]->id);
  EXPECT_EQ(42, static_cast<const Health&>(*objects[1]).value());
}
