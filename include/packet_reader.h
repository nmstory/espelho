#pragma once

#include <algorithm>
#include <optional>
#include <vector>

#include <replicable.h>
#include <stream.h>
#include <type_registry.h>

struct PacketReader
{
public:
  void read(const uint8_t* data,
            size_t len,
            const TypeRegistry& registry,
            std::vector<std::unique_ptr<Replicable>>& objects);

private:
  /* One sequence across all senders: Client::update() doesn't expose the
     sender's identity, so per-peer tracking isn't possible yet. Correct for a
     single peer; nullopt until the first packet, which is always accepted. */
  std::optional<uint16_t> lastSeq {};
};
