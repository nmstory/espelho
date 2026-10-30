# espelho

Object replication and stream serialisation for multiplayer games, built on top of
[Juntos](https://github.com/nmstory/juntos), a UDP transport layer.

The goal is to take live gameplay objects on one machine and mirror their state on
another (_also known as replication_): game code declares which objects are replicable 
and how their fields are serialised, and Espelho handles turning those objects into packets 
that fit under the maximum transmission size, sending them through Juntos, and reconstructing 
them on the receiving side.

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
  newer than the last packet accepted. It currently tracks a single sequence across
  all senders — the transport does not yet expose the sender's identity, so this is
  correct for one peer. Still planned: per-peer sequences, object lifetime
  (create/destroy), and prioritisation of which objects to send when not everything
  fits in one packet.

- **`Espelho` — Facade** (`espelho.h`/`.cpp`) — wraps the Juntos `Client`,
  `PacketWriter`, `PacketReader`, and `TypeRegistry` behind four calls (`AddPeer`,
  `SendObjects`, `Update`, `Objects`), so callers — the console demo and the
  visualiser — never touch packet framing or transport details directly.

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
`Controller` type, and everything lives in one translation unit. A genuine MVC
split — e.g. to support more object types or multiple views — would introduce an
actual `Model` owning local + mirrored objects, a `View` that only issues
`SDL_Renderer` calls, and a thin `Controller` wiring input and the `Espelho` tick
to both.

## Demo

Build, then run two processes that mirror each other's objects over localhost:

```sh
cmake -B build
cmake --build build --parallel

./build/espelho 7000 7001    # terminal 1
./build/espelho 7001 7000    # terminal 2
```

Both processes share a world of 1,000 `Position` objects (see
[Discovery](#discovery-initial)). Each claims the ones near its viewpoint, moves them
every tick, and prints how many it owns and how many it has mirrored from its peer.
If you restart one process, restart both — a fresh process starts its sequence
numbers from zero, so the surviving peer would treat its packets as stale — and delete
`world.txt` first.

### Discovery (initial)

The demo's world is described by a shared text file, `world.txt`, with one object
per line (`id x y owner`; owner `0` means free). The first peer to start seeds it
with a deterministic 40×25 grid of 1,000 objects. Each peer's viewpoint circles the
world, and under a cross-process lock (`world.txt.lock`, a directory, since creating
one is atomic) it claims every free object within a fixed radius — first to claim
keeps it — and replicates only the objects it owns. When an owned object falls out of
range its position is written back to the file and it is freed. Delete `world.txt`
between runs: ownership isn't released on exit.

This is deliberately minimal. Known limitations, to be addressed later:

- Mirrored copies are never removed after the owner releases them, and a stale
  mirror copy stays if a peer later claims an object it previously mirrored.
- No hysteresis, so objects on the radius boundary can be claimed and released
  repeatedly.
- A lock directory left by a crashed process must be removed by hand.
- No hot joining, and no authority handoff beyond first-claim-wins.
- The file is read and rewritten in full every tick.

## Visualiser (optional)

An optional SDL2-based window that shows the same replication demo as above, but
draws each peer's 1,000 `Position` objects as a swarm of moving dots instead of
printing counts to stdout. It is off by default; SDL2 is fetched automatically via
CMake's FetchContent only when you explicitly enable it, so nothing changes for the
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

Each window shows your own 1,000 objects (green) moving automatically in a
phyllotaxis spiral and your peer's mirrored objects (orange). Close either
window, press Escape, or Ctrl+C to exit.

## Tests

```sh
ctest --test-dir build --output-on-failure
```
