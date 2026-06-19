#pragma once

#include <stream.h>
#include <replicable.h>
#include <vector>
#include <type_registry.h>

struct PacketReader {
public:
    std::vector<std::unique_ptr<Replicable>> read(const uint8_t* data, size_t len, const TypeRegistry& registry)
    {
        std::vector<std::unique_ptr<Replicable>> result;
        ReadStream stream{data, len};

        while (stream.pos < stream.cap)
        {
            TypeID tid;
            if (!process_pod(stream, tid)) break;

            auto obj = registry.create(tid);
            if (!obj) break;

            if (!process_pod(stream, obj->id)) break;
            if (!obj->process(stream)) break;

            result.push_back(std::move(obj));
        }

        return result;
    }
};