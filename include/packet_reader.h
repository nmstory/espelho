#pragma once

#include <algorithm>
#include <stream.h>
#include <replicable.h>
#include <vector>
#include <type_registry.h>

struct PacketReader {
public:
    std::vector<std::unique_ptr<Replicable>> read(const uint8_t* data, size_t len, const TypeRegistry& registry,
                                                    std::vector<std::unique_ptr<Replicable>>& objects);
};