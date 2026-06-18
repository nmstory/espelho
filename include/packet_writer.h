#pragma once

#include <stream.h>
#include <replicable.h>
#include <config.h>

struct PacketWriter {
    bool write(Replicable& obj)
    {
        size_t saved = stream.pos;
        TypeID tid = obj.typeID();
        if (!process_pod(stream, tid) || !process_pod(stream, obj.id) || !obj.process(stream))
        {
            stream.pos = saved;
            return false;
        }
        return true;
    }

    const uint8_t* data() const { return buf; }
    size_t size() const { return stream.pos; }
    void reset() { stream.pos = 0; }

private:
    uint8_t buf[espelho::config::MTU_BYTES];
    WriteStream stream{buf, espelho::config::MTU_BYTES};
};