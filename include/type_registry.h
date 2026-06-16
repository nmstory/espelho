#pragma once

#include <unordered_map>
#include <memory>

#include <replicable.h>

class TypeRegistry {
public:
    using PointerToFunc = std::unique_ptr<Replicable>(*)();
    void add(TypeID t, PointerToFunc f);
    inline PointerToFunc create(TypeID typeID) const;

private:
    std::unordered_map<TypeID, PointerToFunc> factories;
};
