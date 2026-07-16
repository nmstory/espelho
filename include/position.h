#pragma once

#include <memory>

#include <replicable.h>

struct Position : public Replicable
{
  Position(int x = 0, int y = 0)
      : m_X(x)
      , m_Y(y)
  {
  }

  static std::unique_ptr<Replicable> make()
  {
    return std::make_unique<Position>();
  }

  bool process(WriteStream& s) override
  {
    return process_pod(s, m_X) && process_pod(s, m_Y);
  }

  bool process(ReadStream& s) override
  {
    return process_pod(s, m_X) && process_pod(s, m_Y);
  }

  TypeID typeID() const override { return TypeID::Position; }

  int x() const { return m_X; }

  int y() const { return m_Y; }

  void set(int x, int y)
  {
    m_X = x;
    m_Y = y;
  }

private:
  int m_X = 0, m_Y = 0;
};
