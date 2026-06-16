#include <type_registry.h>

void TypeRegistry::add(TypeID t, PointerToFunc f)
{
    factories[t] = f;
}

TypeRegistry::PointerToFunc TypeRegistry::create(TypeID typeID) const
{
    auto funcItr = factories.find(typeID);
    if(funcItr != factories.end()){
        return funcItr->second;
    }
    return nullptr;
}