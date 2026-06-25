#include <packet_writer.h>

bool PacketWriter::write(Replicable& obj)
{
  size_t saved = stream.pos;
  TypeID tid = obj.typeID();
  if (!process_pod(stream, tid) || !process_pod(stream, obj.id)
      || !obj.process(stream))
  {
    stream.pos = saved;
    return false;
  }
  return true;
}
