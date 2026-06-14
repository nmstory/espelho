#pragma once

#include <replicable.h>
#include <memory>

struct Position : public Replicable {

    static std::unique_ptr<Replicable> make() {
        return std::make_unique<Position>();
    }

    bool process(WriteStream& s) override { return process_pod(s, x) && process_pod(s, y); }
    bool process(ReadStream& s)  override { return process_pod(s, x) && process_pod(s, y); }
    TypeID typeID() const override { return TypeID::Position; }

private:
    float x = 0, y = 0;
};