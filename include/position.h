#pragma once

#include <replicable.h>
#include <memory>

struct Position : public Replicable {

    Position(const int& x, const int& y) : m_X(x), m_Y(y) {}

    static std::unique_ptr<Replicable> make() {
        return std::make_unique<Position>();
    }

    bool process(WriteStream& s) override { return process_pod(s, x) && process_pod(s, y); }
    bool process(ReadStream& s)  override { return process_pod(s, x) && process_pod(s, y); }
    TypeID typeID() const override { return TypeID::Position; }

private:
    int m_X = 0, m_Y = 0;
};