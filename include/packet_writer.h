#pragma once

#include <config.h>
#include <replicable.h>
#include <stream.h>

struct PacketWriter
{
  bool write(Replicable& obj);

  const uint8_t* data() const { return buf; }

  size_t size() const { return stream.pos; }

  void reset() { stream.pos = 0; }

private:
  uint8_t buf[espelho::config::MTU_BYTES];
  WriteStream stream {buf, espelho::config::MTU_BYTES};
};
