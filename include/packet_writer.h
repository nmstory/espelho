#pragma once

#include <config.h>
#include <replicable.h>
#include <stream.h>

struct PacketWriter
{
  PacketWriter() { reset(); }

  bool write(Replicable& obj);

  const uint8_t* data() const { return buf; }

  size_t size() const { return stream.pos; }

  // true once the packet holds more than the sequence header
  bool hasRecords() const { return stream.pos > sizeof(seq); }

  void reset()
  {
    stream.pos = 0;
    process_pod(stream, seq);
    ++seq;
  }

private:
  uint8_t buf[espelho::config::MTU_BYTES];
  WriteStream stream {buf, espelho::config::MTU_BYTES};
  uint16_t seq = 0;
};
