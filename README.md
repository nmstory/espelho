# espelho

Object replication and stream serialisation for multiplayer games, built on top of
[Juntos](https://github.com/nmstory/juntos), a UDP transport layer.

The goal is to take live gameplay objects on one machine and mirror their state on
another (_also known as replication_): game code declares which objects are replicable 
and how their fields are serialised, and Espelho handles turning those objects into packets 
that fit under the maximum transmission size, sending them through Juntos, and reconstructing 
them on the receiving side.

## Milestone 1: Replication Core

This tag marks the first working end-to-end slice: two processes can each own a
`Position` and a `Health`, mirror them to each other over UDP, reject stale or
duplicated packets, and (optionally) watch it happen live in an SDL2 window.
Specifically, this milestone includes:

- Symmetric serialisation streams (`WriteStream`/`ReadStream`) with round-trip tests.
- A `Replicable` interface plus two example types (`Position`, `Health`).
- A self-registering type registry that reconstructs objects from a wire `TypeID`.
- MTU-aware packet assembly that packs records into as few packets as fit.
- Sequence-number tracking that drops stale or duplicated UDP packets.
- An `Espelho` facade tying the above to Juntos, used by both a console demo
  (`src/main.cpp`) and an optional SDL2 visualiser (`visualiser/main.cpp`).
- Unit tests (GoogleTest) covering stream round-trips, replicable round-trips, and
  packet assembly/parsing.

Still open — see "Replication semantics" below: per-peer sequence tracking, object
lifetime (create/destroy), and prioritisation of which objects to send when not
everything fits in one packet.

## Architecture & design patterns

- **Streams** (`stream.h`) — symmetric `WriteStream` and `ReadStream` over a byte
  buffer. Each replicable type implements a single serialisation routine that works in
  both directions, so the read and write formats can never drift apart. Each type's
  `process()` is effectively a pluggable serialisation strategy plugged into the
  `Replicable` interface, in the spirit of the **Strategy pattern**.

- **Replicable interface** (`replicable.h`) — the base class for anything that can be
  mirrored across the network. A type provides its wire `TypeID`, a factory, and a
  `process()` implementation that serialises its fields.

- **Type registry — self-registering Factory** (`type_registry.h`) — maps a `TypeID`
  read off the wire back to a factory function that constructs the type. Types
  register their own factory at startup (`Espelho::RegisterAllTypes`), so the receive
  path never needs central dispatch code to know about a new type — adding one never
  touches shared code. This replaced an earlier **Visitor pattern** prototype, kept for
  reference in [`alternative-designs/position`](alternative-designs/position/README.txt),
  which required editing a shared `PacketVisitor` interface and `deserialise()` switch
  every time a type was added.

- **Packet assembly** (`packet_writer.h`, `packet_reader.h`) — packs as many object
  records (`TypeID`, object ID, payload) as fit within a conservative MTU budget
  (`config.h`), rolling back any record that does not fit so it can be sent in the
  next packet.

- **Sequence numbers** (`sequence.h`) — a wraparound-aware comparison over the
  circular `uint16_t` sequence space, so the receiver can always tell "newer" from
  "older" even after the counter wraps past 65535, and drop anything that is not
  newer than the last packet accepted.

- **`Espelho` — Facade** (`espelho.h`/`.cpp`) — wraps the Juntos `Client`,
  `PacketWriter`, `PacketReader`, and `TypeRegistry` behind four calls (`AddPeer`,
  `SendObjects`, `Update`, `Objects`), so callers — the console demo and the
  visualiser — never touch packet framing or transport details directly.

- **Replication semantics** (in progress) — each packet carries a sequence number, and
  the receiver drops any packet that is not newer than the last one accepted, so stale
  or duplicated UDP packets can never roll object state backwards. The receiver
  currently tracks a single sequence across all senders (the transport does not yet
  expose the sender's identity), so it is correct for one peer. Still planned: per-peer
  sequences, object lifetime (create/destroy), and prioritisation of which objects to
  send when not everything fits in one packet.

### Visualiser rendering — not MVC (yet)

The SDL2 visualiser (`visualiser/main.cpp`) does **not** implement a formal MVC
architecture, despite drawing live replicated state to a window. It is currently a
single file of free functions plus one loop:

- The `Replicable` objects held by `main()` (local) and by `Espelho::Objects()`
  (mirrored) act as an implicit **model** — the only state that matters.
- Free functions (`drawGrid`, `drawSquare`, `drawTrail`, `drawHealthBar`, `drawText`,
  `renderFrame`) act as an implicit **view** — they only read state (via `const`
  references) and draw; none of them mutate anything.
- `main()`'s loop is an implicit **controller** — it polls SDL events, advances the
  tick timer, drives `Espelho::SendObjects`/`Update`, and calls `renderFrame`.

The separation of concerns is real, but informal: there's no `Model`, `View`, or
`Controller` type, and everything lives in one translation unit. A future milestone
that wants a genuine MVC split — e.g. to support more object types or multiple
views — would introduce an actual `Model` owning local + mirrored objects, a `View`
that only issues `SDL_Renderer` calls, and a thin `Controller` wiring input and the
`Espelho` tick to both.

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

## Visualiser (optional)

An optional SDL2-based window that shows the same replication demo as above, but
draws each peer's `Position` as a moving square instead of printing coordinates to
stdout. It is off by default; SDL2 is fetched automatically via CMake's
FetchContent only when you explicitly enable it, so nothing changes for the
default build.

Building it from source pulls in SDL2's own platform build prerequisites — on
Linux, install something like:

```sh
sudo apt-get install libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
    libxinerama-dev libxss-dev libxkbcommon-dev libwayland-dev libgl1-mesa-dev
```

Then:

```sh
cmake -B build -DESPELHO_BUILD_VISUALISER=ON
cmake --build build --parallel

./build/visualiser/espelho_visualiser 7000 7001    # terminal 1
./build/visualiser/espelho_visualiser 7001 7000    # terminal 2
```

Each window shows your own position (green) moving automatically and your
peer's mirrored position (orange). Close either window, press Escape, or
Ctrl+C to exit.

## Tests

```sh
ctest --test-dir build --output-on-failure
```
