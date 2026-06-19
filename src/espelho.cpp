#include <espelho.h>

#include <position.h>

Espelho::Espelho()
{
    client.init("127.0.0.1", 20001);

    RegisterAllTypes();
}

void Espelho::Update() 
{
    // check for anything to be received
    std::optional<std::vector<uint8_t>> recv = client.update();

    if(recv)
    {
        std::vector<std::unique_ptr<Replicable>> deserialisedObjects = reader.read(recv->data(), recv->size(), typeRegistry);
    }
}

void Espelho::SendObjects(std::vector<Replicable*>& objects)
{
    for (Replicable* obj : objects)
    {
        if (!writer.write(*obj)) // false: not enough space in current packet to write
        {
            Send(writer.data(), writer.size());
            writer.reset();
            writer.write(*obj);
        }
    }
    if (writer.size() > 0)
    {
        Send(writer.data(), writer.size());
        writer.reset();
    }
}

void Espelho::Send(const uint8_t* data, size_t len)
{
    client.send(data, len);
}

void Espelho::RegisterAllTypes(){
    typeRegistry.add(TypeID::Position, &Position::make);
}