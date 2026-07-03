#include <span>
#include <stdexcept>

#include <espelho.h>
#include <health.h>
#include <position.h>

Espelho::Espelho(const int& port)
{
  if (!client.init("127.0.0.1", port)) {
    throw std::runtime_error("Espelho: failed to initialize client");
  }

  RegisterAllTypes();
}

void Espelho::Update()
{
  // check for anything to be received
  std::optional<std::vector<uint8_t>> recv = client.update();

  if (recv) {
    reader.read(recv->data(), recv->size(), typeRegistry, objects);
  }
}

void Espelho::SendObjects(std::vector<std::unique_ptr<Replicable>>& objects)
{
  for (auto& obj : objects) {
    if (!writer.write(
            *obj))  // false: not enough space in current packet to write
    {
      Send(writer.data(), writer.size());
      writer.reset();
      writer.write(*obj);
    }
  }
  if (writer.size() > 0) {
    Send(writer.data(), writer.size());
    writer.reset();
  }
}

void Espelho::Send(const uint8_t* data, size_t len)
{
  client.send(std::span<const uint8_t>(data, len));
}

void Espelho::RegisterAllTypes()
{
  typeRegistry.add(TypeID::Position, &Position::make);
  typeRegistry.add(TypeID::Health, &Health::make);
}
