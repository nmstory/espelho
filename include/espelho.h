#pragma once

#include <packet_reader.h>
#include <packet_writer.h>
#include <submodules/juntos/include/client.h>
#include <type_registry.h>

class Espelho
{
public:
  Espelho(const int& port);

  void RegisterAllTypes();
  void Update();
  void SendObjects(std::vector<std::unique_ptr<Replicable>>& objects);

private:
  void Send(const uint8_t* data, size_t len);

  Client client {};
  PacketWriter writer {};
  PacketReader reader {};
  TypeRegistry typeRegistry {};

  std::vector<std::unique_ptr<Replicable>> objects {};
};
