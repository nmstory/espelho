#pragma once

#include <packet_writer.h>
#include <packet_reader.h>
#include <type_registry.h>
#include <submodules/juntos/include/client.h>

class Espelho 
{
public:
    Espelho(const int& port);

    void RegisterAllTypes();
    void Update();
    void SendObjects(std::vector<Replicable*>& objects);

private:
    void Send(const uint8_t* data, size_t len);

    Client client{};
    PacketWriter writer{};
    PacketReader reader{};
    TypeRegistry typeRegistry{};
};