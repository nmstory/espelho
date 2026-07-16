#pragma once

#include <cstdint>
#include <memory>

#include <replicable.h>

struct Health : public Replicable
{
  Health(uint8_t value = 100)
      : m_Value(value)
  {
  }

  static std::unique_ptr<Replicable> make()
  {
    return std::make_unique<Health>();
  }

  bool process(WriteStream& s) override { return process_pod(s, m_Value); }

  bool process(ReadStream& s) override { return process_pod(s, m_Value); }

  TypeID typeID() const override { return TypeID::Health; }

  uint8_t value() const { return m_Value; }

private:
  uint8_t m_Value = 100;
};
