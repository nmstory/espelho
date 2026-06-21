#pragma once

#include <algorithm>
#include <stream.h>
#include <replicable.h>
#include <vector>
#include <type_registry.h>

struct PacketReader {
public:
    std::vector<std::unique_ptr<Replicable>> read(const uint8_t* data, size_t len, const TypeRegistry& registry,
                                                    std::vector<std::unique_ptr<Replicable>>& objects)
    {
        std::vector<std::unique_ptr<Replicable>> result;
        ReadStream stream{data, len};

        while (stream.pos < stream.cap)
        {
            TypeID tid;
            if (!process_pod(stream, tid)) break;

            
            int id;
            if (!process_pod(stream, id)) break;
            
            auto objItr = std::find_if(objects.begin(), objects.end(),
            [&](const std::unique_ptr<Replicable>& obj) {
                return obj->typeID() == tid && obj->id == id;
            });
            
            if (objItr != objects.end())
            {
                if (!(*objItr)->process(stream)) break; // deserialise in-place
            }
            else
            {
                auto obj = registry.create(tid); // object not recognised, create new
                if (!obj) break;
                obj->id = id;
                if (!obj->process(stream)) break;
                
                result.push_back(std::move(obj));
            }

        }

        return result;
    }
};