#include <type_registry.h>

void TypeRegistry::add(TypeID t, PointerToFunc f)
{
    factories[t] = f;
}

std::unique_ptr<Replicable> TypeRegistry::create(TypeID t) const 
{
    auto it = factories.find(t);
    if (it == factories.end()) return nullptr;
    return it->second();
}