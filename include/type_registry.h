#include <unordered_map>
#include <memory>

#include <replicable.h>

class TypeRegistry {
    std::unordered_map<TypeID, std::unique_ptr<Replicable>(*)()> factories;
public:
    void add(TypeID t, std::unique_ptr<Replicable>(*f)()) { factories[t] = f; }
    std::unique_ptr<Replicable> create(TypeID t) const;   // nullptr if unknown
};
