Visitor Pattern Design (Alternative)
=====================================

This directory is a self-contained snapshot of an early design for espelho's
object replication system, kept here as a record of the design process.

Approach
--------
Incoming packets are dispatched using the Visitor pattern. A PacketVisitor
interface declares a visit() overload per replicable type, and a switch
statement in deserialise() routes each packet to the correct overload based
on its TypeID.

Why it was replaced
-------------------
The visitor pattern requires central dispatch code to be updated every time a
new replicable type is added. In practice, a developer introducing a new type
would need to modify deserialise() and PacketVisitor — code outside the scope
of their own type. This makes the system harder to extend and couples
unrelated types together.

The current design uses a type registry with typed read/write streams. Each
type registers itself independently, so adding a new type is self-contained
and existing dispatch logic is untouched. This also makes versioning cleaner,
since each type owns its own serialisation behaviour.
