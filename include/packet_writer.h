#pragma once

#include <stream.h>
#include <replicable.h>
#include <config.h>

struct PacketWriter {
    void write(Replicable& obj)
    {
        size_t saved = stream.pos;
        TypeID tid = obj.typeID();
        if (!process_pod(stream, tid) || !process_pod(stream, obj.id) || !obj.process(stream))
        {
            stream.pos = saved; // rollback — record didn't fit
            
            // buffer full: flush and retry
        }
    }

private:
    uint8_t buf[espelho::config::MTU_BYTES];
    WriteStream stream{buf, espelho::config::MTU_BYTES};
};