#pragma once

#include <algorithm>
#include <vector>

#include <replicable.h>
#include <stream.h>
#include <type_registry.h>

struct PacketReader
{
public:
  void read(
      const uint8_t* data,
      size_t len,
      const TypeRegistry& registry,
      std::vector<std::unique_ptr<Replicable>>& objects);
};
