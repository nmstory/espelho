# espelho
Object replication and stream serialisation for multiplayer games, built on top of
[Juntos](https://github.com/nmstory/juntos), a UDP transport layer.

The goal is to take live gameplay objects on one machine and mirror their state on
another (_also known as replication_): game code declares which objects are replicable 
and how their fields are serialised, and Espelho handles turning those objects into packets 
that fit under the maximum transmission size, sending them through Juntos, and reconstructing 
them on the receiving side.

## Planned architecture

- **Streams** (`stream.h`) — symmetric `WriteStream` and `ReadStream` over a byte
  buffer. Each replicable type implements a single serialisation routine that works in
  both directions, so the read and write formats can never drift apart.

- **Replicable interface** (`replicable.h`) — the base class for anything that can be
  mirrored across the network. A type provides its wire `TypeID`, a factory, and a
  `process()` implementation that serialises its fields.

- **Type registry** (`type_registry.h`) — maps a `TypeID` read off the wire back to the
  type that owns it. Types register at startup; the receive path looks up the ID and
  asks the registry to construct the object, so adding a new type never touches central
  dispatch code.

- **Packet assembly** (`packet_writer.h`, `packet_reader.h`) — packs as many object
  records (`TypeID`, object ID, payload) as fit within a conservative MTU budget,
  rolling back any record that does not fit so it can be sent in the next packet.

- **Replication semantics** (in progress) — each packet carries a sequence number
  (`sequence.h`), and the receiver drops any packet that is not newer than the last
  one accepted, so stale or duplicated UDP packets can never roll object state
  backwards. The receiver currently tracks a single sequence across all senders (the
  transport does not yet expose the sender's identity), so it is correct for one peer.
  Still planned: per-peer sequences, object lifetime (create/destroy), and
  prioritisation of which objects to send when not everything fits in one packet.

## Demo

Build, then run two processes that mirror each other's objects over localhost:

```sh
cmake -B build
cmake --build build --parallel

./build/espelho 7000 7001    # terminal 1
./build/espelho 7001 7000    # terminal 2
```

Each process moves its own `Position` every tick and prints the mirrored objects it
receives from its peer. If you restart one process, restart both — a fresh process
starts its sequence numbers from zero, so the surviving peer would treat its packets
as stale.

## Tests

```sh
ctest --test-dir build --output-on-failure
```

