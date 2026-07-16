#pragma once

#include <chrono>
#include <optional>

#include <client.h>
#include <packet_reader.h>
#include <packet_writer.h>
#include <type_registry.h>

class Espelho
{
public:
  Espelho(const int& port,
          std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt);

  [[nodiscard]] bool AddPeer(const std::string& hostname, int port);

  void RegisterAllTypes();
  void Update();
  void SendObjects(std::vector<std::unique_ptr<Replicable>>& objects);

  const std::vector<std::unique_ptr<Replicable>>& Objects() const
  {
    return objects;
  }

private:
  void Send(const uint8_t* data, size_t len);

  Client client {};
  PacketWriter writer {};
  PacketReader reader {};
  TypeRegistry typeRegistry {};

  std::vector<std::unique_ptr<Replicable>> objects {};
};
