#pragma once

#include <memory>
#include <unordered_map>

#include <replicable.h>

class TypeRegistry
{
public:
  using PointerToFunc = std::unique_ptr<Replicable> (*)();
  void add(TypeID t, PointerToFunc f);
  std::unique_ptr<Replicable> create(TypeID t) const;

private:
  std::unordered_map<TypeID, PointerToFunc> factories;
};
