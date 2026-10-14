# OpenGL Engine — Architecture Map & Game-First Roadmap

**Status:** Living architecture baseline  
**Version:** 1.1  
**Last updated:** 2026-10-19  
**Primary priority:** Make a real, playable game as early as possible, then grow the engine in response to actual game needs.

> This document describes the intended architecture and development plan. It does not claim that all listed systems exist in the current repository. Keep implementation status visible in issues or a separate progress checklist. Update this document when a meaningful architecture decision changes.

---

## 1. Product vision

Build a C++-native, code-first engine for **2D games first, with focused 2.5D capabilities added when a real game needs them**. The experience should borrow useful Unreal Engine ideas—an engine-owned runtime, lifecycle hooks, a game instance, worlds, game modes, actors and components—without copying Unreal's scale, editor dependency, reflection system, or deep inheritance patterns.

The engine's main purpose is not to be an impressive framework on paper. Its purpose is to help us make games.

### Product goals

- Get to a playable 2D game early, while the engine is still small.
- Keep a tiny playable sample game working throughout development.
- Let the engine own startup, the frame loop, timing, event processing, rendering orchestration, and shutdown.
- Let game code define worlds and gameplay in C++, without a mandatory visual level editor.
- Use composition as the default; keep Actor inheritance optional and shallow.
- Keep GLFW/OpenGL implementation details out of game-facing headers.
- Build the engine as a **shared library** (`engine.dll` on Windows) with a deliberate public API boundary.
- Use Dear ImGui for runtime debugging and inspection—not as a required world-authoring tool.
- Add systems when the current game or a near-term milestone needs them.
- Keep each milestone buildable, testable, and playable where practical.

### Non-goals for the initial engine

- A general-purpose 3D engine.
- A full Unreal-like editor, visual scripting, or reflection/garbage-collection framework.
- Networking, a plugin marketplace, or hot reload before a concrete need appears.
- Building every planned subsystem before the first game can be played.
- Premature abstractions that have no current user in the sample game.

---

## 2. The game-first rule

**At every stage, prefer the smallest engine change that lets the game become more real.**

We will not finish a large abstract framework and only then start making a game. Instead, we will develop the engine alongside a small reference game and use that game to reveal what the engine actually needs.

### The reference game

Use a small side-scrolling platformer as the recurring test project. Working title: **Shadow Runner**.

The first version needs only:
- A window and a reliable engine-owned game loop.
- Keyboard input.
- A simple player represented by a rectangle or placeholder sprite.
- Horizontal movement and jumping.
- A few platforms and basic collision.
- A camera or view that can show the play area.
- A restart/exit path and a clear way to build and run the game.

Later, the same game can grow to include sprites, animation, enemies, pickups, sound, multiple levels, save data, parallax, lighting, and selected 3D objects.

The game is a **vertical slice**: each new engine capability should be exercised by real gameplay instead of a disconnected technical demo.

### Rules for avoiding overengineering

1. Do not build an advanced Actor/Component system before the prototype proves which lifecycle and ownership features it needs.
2. Do introduce the intended extension points early enough that game code does not take ownership of the engine loop.
3. Prefer a simple, explicit C++ API over reflection, macros, automatic discovery, or hidden magic.
4. Do not add a generic asset database, prefab system, scene serializer, editor, or plugin framework until a real workflow justifies it.
5. If a feature can be implemented cleanly in the game first and is not yet reusable, keep it in game code. Promote it into the engine after a second use case or a clear cross-cutting need appears.
6. Keep the reference game working after every milestone; do not let architecture refactors strand it for weeks.

---

## 3. Architecture map

The engine owns execution. The game supplies configuration and gameplay behavior through stable extension points.

```text
                         GAME EXECUTABLE
                  (Shadow Runner / user's game)
                                |
                                v
                     Game entry / game factory
                                |
                                v
+----------------------------------------------------------------+
|                    ENGINE RUNTIME (DLL)                        |
|                                                                |
|  EngineRuntime                                                  |
|  - startup / shutdown                                           |
|  - frame loop and frame timing                                  |
|  - event pump and input-frame orchestration                     |
|  - update / render scheduling                                   |
|  - orderly teardown                                            |
|                                                                |
|  GameInstance                                                   |
|  - state that survives world changes                            |
|  - game-wide configuration/services when needed                 |
|                                                                |
|  World + GameMode                                               |
|  - current level/world                                          |
|  - world rules and initial setup                                |
|  - actor ownership and world lifecycle                          |
|                                                                |
|  Actors + Components                                            |
|  - player, enemies, platforms, pickups, doors                   |
|  - reusable capabilities and gameplay behavior                  |
|                                                                |
|  Engine services (introduced as needed)                         |
|  - Window / Platform Events / Input                             |
|  - 2D Renderer / Camera / Assets                                |
|  - Collision / Physics / Animation / Audio                     |
|  - Debug tools (optional development layer)                     |
+----------------------------------------------------------------+
                                |
                                v
                  Platform + graphics backends
                   GLFW / OpenGL / OS facilities
```

This is a logical architecture, not a requirement that each box become a separate DLL or CMake target. Initially, keep one `engine` DLL and one game executable. Split targets only when there is a real dependency, testing, or build benefit.

### Dependency direction

```text
Game code
   -> Public gameplay framework
      -> Public engine service interfaces
         -> Private implementations
            -> GLFW / OpenGL / OS / third-party libraries
```

Game code should not call GLFW directly, manipulate OpenGL buffers, or own the runtime loop. Rendering and platform internals should be replaceable without rewriting gameplay.

---

## 4. Core concepts and intended responsibilities

### `EngineRuntime`

The runtime is the process-level orchestrator. It initializes services, owns the main loop, coordinates frame timing and system updates, and shuts down in a deterministic order. Game code configures the runtime through a small entry point; it does not replace the main loop.

### `GameInstance`

One per running game instance. It owns game-wide state that must survive a world/level change, such as settings or a run/session manager if the game needs one. It should not become a global container for every service.

### `World`

Represents the active level/simulation context. It owns its actors and world-scoped state. The first version can create worlds entirely in C++ through level-builder functions. Serialization and editor-authored scenes are not prerequisites.

### `GameMode`

Defines the rules and initial setup for a world/session: spawning the player, selecting rules, setting up initial actors, and responding to game-specific lifecycle events. It should coordinate gameplay, not contain every player's, enemy's, and pickup's implementation.

### `Actor`

A world object with identity and a documented lifecycle. Examples: player, enemy, platform, pickup, door, checkpoint. Custom Actor subclasses are allowed when they make gameplay clearer, but should not be required for every object.

### `Component`

A reusable capability attached to an Actor, such as movement, health, sprite rendering, collision, or audio. Composition is preferred over long inheritance trees. Components need clear ownership, initialization order, and safe destruction semantics.

### Engine services

Introduce services in response to game needs. Likely services include:
- Window and platform events.
- Action-based input.
- 2D rendering, camera and texture resources.
- Collision and platformer movement; a physics backend only if needed.
- Animation and audio.
- Asset loading/caching.
- Debug inspection and profiling.

Do not create a service abstraction for every small class by default. Keep the public API focused.

---

## 5. Planned game-facing experience

The following examples show the *direction* of the API. They are sketches, not guaranteed current signatures.

### Game entry

```cpp
int main()
{
    return Engine::Run<ShadowRunnerGame>();
}
```

The engine creates the runtime and invokes the game entry/lifecycle. The actual factory, error reporting and return type will be decided when implementing the runtime.

### Game setup

```cpp
class ShadowRunnerGame final : public Engine::GameInstance
{
public:
    void OnStart() override
    {
        // Select or create the initial world.
        // Set game-wide configuration if needed.
    }
};
```

### Code-authored world

```cpp
class RuinedCityMode final : public Engine::GameMode
{
public:
    void StartPlay() override
    {
        World().SpawnActor<PlayerCharacter>();
        World().SpawnActor<PatrolEnemy>();
        World().SpawnActor<HealthPickup>();
    }
};
```

### Gameplay object

```cpp
class PlayerCharacter final : public Engine::Actor
{
public:
    void BeginPlay() override;
    void Tick(float deltaTime) override;
};
```

### Composition

A player may combine Transform, Movement, Sprite/Animation, Collision, and Health components. An enemy can reuse Health, Sprite, and Collision while using different AI/movement behavior.

The first implementation should use ordinary C++ and explicit ownership. Do not introduce Unreal-style garbage collection or reflection unless later requirements demonstrate a compelling need.

---

## 6. Game-first development roadmap

The order is intentionally different from a framework-first plan. **We will create something playable before completing the full engine framework.**

### Milestone 0 — Reliable build and a clear entry point

**Build:**
- Clean CMake configure/build for the current engine DLL and game executable.
- Stable public/private include boundaries.
- A minimal game entry point.
- Engine startup and shutdown with logging/errors.
- A reliable window and engine-owned loop.

**Game result:** pressing Run opens a window, runs through the engine runtime, and exits cleanly.

**Do not build yet:** advanced actors, physics engine, asset database, animation system, editor.

**Exit criteria:** clean Debug build and a repeatable launch/shutdown path.

### Milestone 1 — First playable prototype as early as possible

**Build only what the game needs:**
- Keyboard/action input.
- Frame delta and basic timing.
- A simple 2D draw path for colored rectangles or one placeholder texture.
- A minimal camera/view if necessary.
- A simple game-owned prototype update path inside the engine's extension points.
- Basic movement and platform collision; use the simplest correct approach for the first prototype.

**Game result:** a controllable character moves and jumps across platforms. The game loop is owned by the engine, not by `main.cpp` in the game project.

**Important:** do not block this milestone on a perfect Actor/Component design. The prototype should use the smallest gameplay API that respects the runtime boundary. Then use it to inform the framework.

**Exit criteria:** a person can launch the game and play the tiny level; input, timing, and basic collision behave consistently.

### Milestone 2 — Add the minimum gameplay framework that the prototype needs

**Build:**
- `GameInstance`, `World`, and `GameMode`.
- `Actor` identity and a clear lifecycle (`BeginPlay`, `Tick` or equivalent, `EndPlay`).
- Component ownership and lifecycle.
- Safe spawn/destroy behavior when requested during an update.
- A simple C++ level builder.

**Game result:** the player, platforms, and a simple enemy are represented as world-owned actors. Reusable behavior moves into components where that actually reduces duplication.

**Exit criteria:** a world can start, update, and shut down; actor/component lifetime tests pass; the prototype still plays.

### Milestone 3 — Make a small complete 2D game

Grow the reference game into a short but complete experience:
- Sprite and texture loading.
- Camera follow and draw order.
- Basic 2D collision/platformer movement.
- One enemy type, one pickup, one hazard or checkpoint.
- Simple animation and audio as required.
- Restart and level transition.
- A small amount of game UI if needed.
- A straightforward way to define additional levels in C++.

**Game result:** a tiny but complete game with a start, challenge, success/failure state, and restart path.

**Exit criteria:** someone other than the author can run and finish the game from a clean build. The game is fun enough to expose workflow friction.

### Milestone 4 — Improve developer workflow based on actual friction

Add only tools that solve observed problems:
- ImGui actor/component inspector.
- Collision-shape and transform overlays.
- Frame-time and renderer statistics.
- Input-action diagnostics.
- Useful logs and error messages.
- Asset path/configuration improvements.
- Automated tests for lifecycle and core logic.

**Game result:** gameplay is easier to debug and iterate on; debug UI is optional and not needed to author a level.

**Exit criteria:** common bugs can be diagnosed without inserting temporary print statements everywhere.

### Milestone 5 — Production basics and standalone distribution

**Build:**
- Predictable Debug and Release configurations.
- Packaging of the game executable, `engine.dll`, required third-party DLLs and assets.
- Error handling for missing resources.
- A documented game template or starter project if the workflow is stable.
- A clean-checkout build/run guide.

**Game result:** the game can be copied to another supported Windows machine/environment and launched with all required files.

**Exit criteria:** clean package works without relying on developer-only paths or build-directory accidents.

### Milestone 6 — Focused 2.5D features, driven by a real game

Only when a game needs them, add:
- Explicit depth and layer/order policies.
- Parallax and multi-plane backgrounds.
- Orthographic camera improvements and perspective support if justified.
- Selected 3D props or meshes.
- Appropriate lighting and shadows.
- Selected 3D collision/physics features if required.

**Game result:** a small scene demonstrates the intended 2.5D look while the same runtime and gameplay framework remain intact.

**Exit criteria:** a representative 2.5D scene works without redesigning the core runtime or world ownership model.

### Milestone 7 — Grow by game demand, not by checklist

Possible later features:
- Save/load.
- Data-driven actor templates or prefab-like definitions.
- Tilemap tooling or code-assisted tilemaps.
- More animation features.
- More advanced physics.
- Controller support and configurable bindings.
- Better profiling and asset diagnostics.
- Additional platforms only when explicitly targeted.

Every feature needs a real use case, a small design, tests where practical, and a game/demo that exercises it.

---

## 7. Feature policy: build now, build soon, defer

| Priority | Features | Why |
|---|---|---|
| **Now** | Build reliability, engine-owned loop, window, input, basic timing, simple 2D draw path | Required to make the first playable prototype |
| **Soon** | Minimal World/Actor lifecycle, collision, player movement, camera, basic textures | Required to make the prototype into a real small game |
| **When the game needs it** | Components beyond immediate needs, animation, audio, level transitions, ImGui inspection, resource caching | Add when they improve the next playable milestone |
| **Later** | 2.5D lighting, selected 3D props, perspective, advanced physics, save/load, serialization | Avoid delaying the first game |
| **Not planned by default** | Full visual editor, visual scripting, general-purpose 3D engine, reflection/GC framework, plugin ecosystem | Outside the focused product scope unless a strong need emerges |

This table is a prioritization guide, not a promise that a feature will never be implemented.

---

## 8. Shared-library and CMake policy

The accepted deployment model is a shared engine library (`SHARED`; `engine.dll` on Windows) plus a game executable.

Conceptual build graph:

```text
game (executable)
   └── links Engine::Engine

engine (SHARED library / DLL)
   ├── core and runtime
   ├── platform/window
   ├── input
   ├── rendering and gameplay as they are implemented
   └── private third-party implementation dependencies
```

Keep one engine target and one game target initially. Do not split every subsystem into a separate target. Add tests as needed.

### DLL requirements

- Define a single `ENGINE_API` macro that expands to `__declspec(dllexport)` while building the engine and `__declspec(dllimport)` for consumers on Windows; provide a sensible alternative for other supported compilers/platforms.
- Apply the export macro consistently to public classes/functions that cross the DLL boundary.
- Avoid exposing STL-heavy implementation details or third-party types in the public API without deliberate ABI consideration.
- Keep implementation-only dependencies private when public headers do not require them.
- Decide which runtime DLLs and assets must ship with the game; test packaging from a clean output directory.
- Treat the public C++ API as a source-level interface initially. Binary compatibility across arbitrary compiler versions is not guaranteed unless we explicitly design and test for it.

### Include and target boundaries

Public game-facing headers belong under `engine/include/engine/`. Implementation belongs under `engine/src/` (or the repository's current equivalent after audit). Public headers should not require consumers to include engine-private paths.

Use target-scoped CMake settings. Do not add global include paths or compile definitions merely to make one source file compile. Do not mass-move files until CMake, includes, exports, and builds can be changed together and verified.

---

## 9. Runtime frame/lifecycle direction

The exact sequence will be finalized when collision/physics and rendering are implemented, but the runtime will own orchestration. A provisional sequence:

1. Initialize platform/window and required services.
2. Create the game instance and initial world.
3. Poll platform events and update input state.
4. Calculate frame delta and apply configured frame/timing limits.
5. Run game/world updates.
6. Step physics at a defined cadence if a physics system exists.
7. Synchronize transforms and collision state where needed.
8. Render the world and optional debug overlays.
9. Present the frame and finalize deferred world changes/resources at safe points.
10. On shutdown, stop the world/game and release services in reverse dependency order.

Lifecycle contracts must cover spawn/destroy during update, world changes, minimized windows, and shutdown. Fixed-step physics should be introduced if/when required, without forcing all gameplay updates to use a fixed timestep.

---

## 10. 2D and 2.5D scope

### 2D baseline

Prioritize the systems needed by a small complete game: window/runtime, input, 2D rendering, transforms, camera, textures, actors/world, collision/movement, simple animation/audio where required, level setup and packaging.

### 2.5D extension

Add depth, layers, parallax, selected 3D assets and lighting based on a representative game. Preserve the 2D path as a first-class use case. Do not build a universal 3D renderer or full 3D physics stack without a concrete target.

---

## 11. Testing and quality gates

- Keep the reference game building and playable at each milestone.
- Test pure utilities and input-state/action logic where feasible.
- Test world/actor/component lifecycle, deferred destruction, and world transitions.
- Build Debug and Release from a clean checkout.
- Keep compiler warnings enabled and fix meaningful warnings.
- Add window/render smoke tests where CI/display constraints permit.
- Test the packaged game from a clean directory, not only from the IDE build tree.
- Use profiling and platform-appropriate diagnostics when available.

A milestone is not complete just because its API compiles. It is complete when the reference game uses the feature successfully and important behavior is tested.

---

## 12. Decision register

| Topic | Current direction | Status |
|---|---|---|
| Product scope | 2D first; focused 2.5D later | Agreed direction |
| Main priority | Make playable games early; evolve engine alongside them | **Primary rule** |
| Authoring workflow | C++ code-first; no mandatory editor | Agreed direction |
| Debug UI | ImGui for runtime inspection/debugging only | Agreed direction |
| Runtime | Engine owns main loop and lifecycle | Agreed direction |
| Gameplay model | GameInstance, World, GameMode, Actor, Component concepts | Direction agreed; APIs not frozen |
| Composition/inheritance | Composition by default; optional shallow Actor subclasses | Agreed direction |
| Engine deployment | `engine` shared library / DLL + game executable | **Current decision** |
| Initial CMake structure | One engine target, one game target; tests as needed | Agreed direction |
| First game | Small 2D platformer as vertical slice | Proposed reference project |
| Physics backend | Choose after the first playable collision requirements are clear | Deferred |
| Asset serialization | Add when the game workflow needs it | Deferred |
| Hot reload/plugins/editor | Not initial requirements | Deferred |
| Full 2.5D feature set | Driven by a representative game | Deferred |

---

## 13. How we change architecture safely

1. Inspect the actual repository and build graph before changing code.
2. Identify the smallest change that advances the current playable milestone.
3. Update code, CMake, tests, and this document together when a decision changes.
4. Mark decisions as agreed, proposed, or deferred.
5. Keep implementation status separate from architecture intent.
6. Preserve a playable reference game during refactors.
7. Prefer a small, tested working slice over a large unfinished subsystem.

---

## Immediate next steps

1. Audit the current repository and verify the actual engine DLL, game executable, public headers, and CMake target settings.
2. Make sure the game entry point hands control to an engine-owned runtime rather than implementing its own frame loop.
3. Get a minimal window + input + simple 2D draw path running.
4. Make the first controllable character and platform prototype playable.
5. Only then firm up the Actor/Component API based on what that prototype needs.
6. Keep the prototype alive as the reference game while adding features.

**Guiding principle:** the engine is successful when it helps us make games—not when every planned subsystem exists.
