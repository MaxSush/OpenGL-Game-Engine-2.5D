# OpenGL Engine — Architecture Map & Game-First Roadmap

**Status:** Living architecture baseline  
**Version:** 1.2  
**Last updated:** 2026-10-20  
**Primary priority:** Make a real, playable 2D game as early as possible, then grow the engine in response to actual game needs.

> This document describes intended architecture and the development plan. It does not claim that every listed system exists. **Implementation status lives in [`STATUS.md`](STATUS.md).** Update this document only when an architecture decision changes.

### Changes in 1.2

- `Game` + `Engine::Run(game, config)` replaces `GameInstance`/`GameMode` as the M0/M1 gameplay entry point (implemented).
- `GameInstance`, `World`, `GameMode` are now **deferred**; a plain `Scene` is preferred until persistent cross-scene state or multiple worlds are needed.
- Actor/Component is marked **unproven**; the model is chosen after the prototype (see 4).
- Shared library kept deliberately (future hot-reload), with an explicit list of the costs we accept (see 8).
- M1 split into M1a (loop, time, renderer) and M1b (player, collision). Debug UI moved earlier.
- Input subsystem is declared **feature-frozen** until the game needs more.

---

## 1. Product vision

Build a C++-native, code-first engine for **2D games first, with focused 2.5D capabilities added when a real game needs them**. Borrow useful Unreal ideas (engine-owned runtime, lifecycle hooks, composition) without copying its scale, editor dependency, reflection system, or deep inheritance.

The engine's purpose is to help us make games, not to be an impressive framework on paper.

### Product goals

- Reach a playable 2D game early, while the engine is still small.
- Keep a tiny playable sample game working throughout development.
- The engine owns startup, frame loop, timing, event processing, rendering orchestration and shutdown.
- Game code defines gameplay in C++; no mandatory editor.
- Prefer composition; keep any inheritance optional and shallow.
- Keep GLFW/OpenGL out of game-facing headers.
- Build the engine as a **shared library** (`engine.dll`) with a deliberate public API boundary.
- Use Dear ImGui for runtime tuning and inspection, not as a world-authoring tool.
- Add systems only when the current game or the next milestone needs them.
- Keep each milestone buildable, testable and playable.

### Non-goals for the initial engine

- A general-purpose 3D engine.
- An Unreal-like editor, visual scripting, or reflection/GC framework.
- Networking, plugin marketplace, or hot-reload *implementation* before a concrete need (the boundary should merely not prevent it).
- Building every planned subsystem before the first game is playable.
- Abstractions without a current user in the sample game (for example an RHI with one backend).

---

## 2. The game-first rule

**At every stage, prefer the smallest engine change that lets the game become more real.**

We develop the engine alongside a small reference game and let that game reveal what the engine needs.

### The reference game

A small side-scrolling platformer. Working title: **Shadow Runner**.

First version:
- Window and an engine-owned game loop.
- Keyboard input.
- A player as a rectangle or placeholder sprite.
- Horizontal movement and jumping.
- A few platforms and basic collision.
- A camera that shows the play area.
- A quit/restart path and a clear build-and-run process.

Later: sprites, animation, enemies, pickups, sound, multiple levels, save data, parallax, lighting, selected 3D objects.

### Rules for avoiding overengineering

1. Do not build an Actor/Component/World framework before the prototype shows which lifecycle and ownership features it needs.
2. Do introduce extension points early enough that game code never owns the loop.
3. Prefer a simple explicit C++ API over reflection, macros or hidden magic.
4. No asset database, prefab system, scene serializer, editor or plugin framework until a real workflow needs one.
5. If a feature can live in game code and is not reusable yet, keep it there. Promote it after a second use or a clear cross-cutting need.
6. Keep the reference game working after every milestone.
7. Freeze subsystems that are already "enough" (currently: input) rather than polishing them ahead of need.

---

## 3. Architecture map

The engine owns execution. The game supplies behavior through stable extension points.

```text
                    GAME EXECUTABLE
              (Shadow Runner / user's game)
                          |
              class MyGame : Engine::Game
              int main() { return Engine::Run(game, config); }
                          |
                          v
+----------------------------------------------------------+
|                  ENGINE RUNTIME (DLL)                    |
|                                                          |
|  Engine::Run  -> log init/shutdown, exception reporting  |
|  Application  -> owns window, input, clock, frame loop   |
|    - poll events -> Input::BeginFrame                    |
|    - OnUpdate(dt)  (once per rendered frame)             |
|    - OnFixedUpdate(step)  (0..N per frame)               |
|    - clear -> OnRender -> swap -> Input::EndFrame        |
|                                                          |
|  Services (added as needed)                              |
|  - Window / Platform events        [implemented]         |
|  - Input (actions, contexts)       [implemented, frozen] |
|  - 2D Renderer / Camera / Textures [next]                |
|  - Collision / Movement helpers    [game code first]     |
|  - Debug UI (ImGui)                [soon]                |
|  - Audio / Animation / Assets      [when needed]         |
+----------------------------------------------------------+
                          |
                          v
            Platform + graphics backends
             GLFW / OpenGL / OS facilities
```

This is a logical architecture, not a promise that each box becomes a separate target. Keep one `engine` target and one `game` target until there is a concrete dependency, testing or build reason to split.

### Dependency direction

```text
Game code
   -> Public engine API (Game, Application, Input, Renderer2D ...)
      -> Private implementations (src/)
         -> GLFW / OpenGL / OS / third-party libraries
```

Game code must not call GLFW, touch OpenGL buffers, or own the runtime loop.

---

## 4. Core concepts

### `Engine::Run` and `Application` (implemented)

`Run` initialises logging, constructs `Application`, converts exceptions to a log entry and exit code, and shuts logging down last. `Application` owns the window, input system and clock and runs the loop. It destroys input before the window, and both before the logger. Nothing throws across the DLL boundary.

### `Game` (implemented)

An abstract class with overridable callbacks: `OnStart`, `OnUpdate`, `OnFixedUpdate`, `OnRender`, `OnResize`, `OnShutdown`. The game registers input actions in `OnStart`; `Application` finalizes the action map afterward. The game never owns the loop.

Contract notes:
- `OnUpdate` samples input intent once per rendered frame.
- `OnFixedUpdate` advances simulation at a constant step and must not read raw per-frame "pressed" input directly; it consumes intent captured in `OnUpdate`.
- `OnResize` is delivered after `OnStart` and never with a zero size.

### `Scene` (planned, M2)

A simple container for the current level's entities and level-scoped state, built in C++. It replaces `World`/`GameMode` until a proven need arises. Level loading is a function that fills a scene.

### Entity model (undecided, M2)

Unreal's `Actor*` is safe because of UObject, garbage collection and reflection. We have deliberately excluded those, so **object lifetime is our real risk**. Decide after the prototype between:

1. Plain structs in game-owned containers, with stable IDs or handles.
2. A small custom handle-based entity store.
3. A small ECS library (for example EnTT).

Whichever is chosen must support safe spawn/destroy during update (deferred destruction) and must not expose raw pointers that can dangle.

### `GameInstance`, `GameMode` (deferred)

In Unreal these exist largely for networking, server authority and level travel. A single-player 2D game does not need them. Reintroduce `GameInstance` only if persistent cross-scene state becomes awkward to hold in `Game`.

---

## 5. Game-facing API (current direction)

```cpp
class PlatformerGame final : public Engine::Game
{
public:
    bool OnStart() override;                 // register actions, load level
    void OnUpdate(float dt) override;        // sample input intent
    void OnFixedUpdate(float step) override; // movement, collision
    void OnRender() override;                // draw via Renderer2D
    void OnShutdown() override;
};

int main()
{
    Engine::AppConfig config;
    config.window.title = "Shadow Runner";

    PlatformerGame game;
    return Engine::Run(game, config);
}
```

Input intent pattern for the player:

```text
OnUpdate:      read actions -> fill Intent{ move_x, jump_buffered }
OnFixedUpdate: consume Intent -> integrate velocity -> resolve collisions
OnRender:      draw using current positions (interpolate only if needed)
```

---

## 6. Game-first development roadmap

### Milestone 0 — Reliable build, entry point, loop (largely done)

- Clean CMake configure/build for engine DLL and game executable.
- Window, input, logging.
- `Game` interface, `Application`, `Engine::Run`, frame timing, fixed-step accumulator.
- Minimize and resize handling, clean shutdown order.

**Result:** pressing Run opens a window, runs through the engine runtime, quits on Esc.  
**Exit criteria:** clean Debug build, repeatable launch/shutdown, no stuck input.

### Milestone 1a — First pixels

- glm dependency (as a submodule).
- Minimal `Renderer2D`: orthographic camera, batched colored quads, embedded shaders. Direct OpenGL calls; no RHI.
- Move `glViewport`/clear out of `Application` into the renderer.

**Result:** colored rectangles on screen, responding to resize.

### Milestone 1b — First playable prototype

- Player controller and platform collision in **game code** (AABB vs rectangles).
- Fixed-step movement, input-intent sampling, jump buffering and coyote time as needed.
- ImGui tuning panel (gravity, jump speed, friction) so movement can be tuned live.

**Result:** a controllable character runs and jumps across platforms. **This is the milestone that matters.**  
**Exit criteria:** someone can launch the game and play the tiny level; movement is framerate independent.

### Milestone 2 — Minimum framework the prototype needs

- Choose the entity model (see 4) based on actual pain from M1b.
- `Scene`, deferred spawn/destroy, a simple C++ level builder.
- Move proven-reusable game code (for example AABB collision) into the engine.
- Unit tests for lifecycle and core logic.

**Exit criteria:** player, platforms and one enemy live in a scene; lifetime tests pass; the prototype still plays.

### Milestone 3 — Small complete 2D game

- Textures/sprites (stb_image) and an asset-path strategy.
- Camera follow and draw order.
- One enemy, one pickup, one hazard or checkpoint.
- Animation and audio as required.
- Restart, level transition, minimal UI.

**Exit criteria:** someone other than the author can run and finish the game from a clean build.

### Milestone 4 — Workflow improvements driven by friction

- Inspector and collision/transform overlays.
- Frame-time and renderer statistics.
- Input diagnostics.
- CI build (Debug + Release) and expanded tests.

### Milestone 5 — Production basics

- Debug and Release configurations, packaging of exe + `engine.dll` + third-party DLLs + assets.
- Missing-resource handling and a clean-checkout build/run guide.

### Milestone 6 — Focused 2.5D features (only if a game needs them)

Depth/layer policy, parallax, perspective camera, selected 3D props, lighting.

### Milestone 7 — Grow by demand

Save/load, data-driven templates, tilemap tooling, controller support, remappable bindings, hot reload (if still wanted), more physics.

---

## 7. Feature policy

| Priority | Features | Why |
|---|---|---|
| **Done** | Window, input, logging, `Game`/`Application` loop, fixed-step timing | Foundation exists |
| **Now** | Renderer2D with ortho camera, glm dependency | Nothing is drawn yet |
| **Soon** | Player and collision in game code, ImGui tuning panel, input tests | Makes the prototype playable and tunable |
| **When the game needs it** | Entity model and Scene, textures, animation, audio, level transitions, resource caching | Add when they improve the next playable milestone |
| **Later** | 2.5D lighting, 3D props, perspective, advanced physics, save/load, serialization, hot reload | Avoid delaying the first game |
| **Not planned** | Full editor, visual scripting, reflection/GC, plugin ecosystem, RHI abstraction | Outside the scope unless a strong need emerges |

---

## 8. Shared-library and CMake policy

**Decision:** keep `engine` as a `SHARED` library plus a game executable. The primary reason is keeping a future hot-reload path open.

### Costs we are consciously accepting

- Export annotations on every public class/function (`ENGINE_API`), and discipline about STL/third-party types in public headers.
- Static spdlog gets linked into both exe and DLL, which works only while they share the same CRT. A hot-reloaded game DLL would add a third copy; solve that with an exported logging function if and when we reach it.
- Unit tests cannot link private classes through the DLL. Test pure logic (`PhysicalInputState`, `InputActionResolver`, `InputEventQueue`) by compiling those `.cpp` files directly into a test executable.
- Binary compatibility across compiler versions is not guaranteed; the API is source-level.

### Revisit trigger

If hot reload is not attempted by the end of Milestone 3, reconsider switching the default to `STATIC` with `SHARED` as a CMake option (`ENGINE_API` gains a third branch that expands to nothing). Unreal itself is modular in the editor and monolithic static in shipping builds.

### DLL requirements

- Single `ENGINE_API` macro: export when building the engine, import for consumers, visibility attribute on GCC/Clang, empty otherwise.
- `CXX_VISIBILITY_PRESET hidden` and `VISIBILITY_INLINES_HIDDEN ON` so Linux builds catch missing exports.
- Small data-only structs read by the game (for example `InputFrame` accessors) either carry `ENGINE_API` or are inline in the header.
- Keep implementation-only dependencies PRIVATE when public headers do not need them.
- Nothing may throw across the DLL boundary; `Engine::Run` is the catch-all.
- Hot-reload shape (not implemented): a game DLL would export one `extern "C"` factory returning `Game*`.

### Include and target boundaries

Public headers live under `engine/include/engine/`. Implementation lives under `engine/src/`. Use target-scoped CMake settings only. Third-party header-only includes are `SYSTEM` so warnings stay readable. Warnings are on (`/W4` or `-Wall -Wextra`) for the engine; the game target should follow.

### Dependencies

glfw and spdlog are submodules. **glm must become a submodule too**; the current `.gitignore` excludes `external/include/glm/*`, which breaks fresh clones.

---

## 9. Runtime frame/lifecycle (implemented)

1. `Run`: init logging, construct `Application`.
2. Create window and input; set the resize callback; set the viewport.
3. `Game::OnStart`; finalize the input action map; deliver the first `OnResize`.
4. Each frame: compute dt (clamped to `max_frame_time`), poll events, `Input::BeginFrame`.
5. If minimized: drain input, sleep briefly, reset the clock, skip game and render.
6. `OnUpdate(dt)`.
7. Fixed-step loop: `OnFixedUpdate(step)` up to `max_fixed_steps_per_frame`; drop any remaining backlog.
8. Clear, `OnRender`, swap, `Input::EndFrame`, update FPS.
9. On exit: `OnShutdown`, destroy input then window, shut down logging.

Lifecycle contract points still to define when scenes arrive: spawn/destroy during update, scene transitions, shutdown order of scene objects. Interpolation alpha is added to `FrameTime` only if movement looks jittery.

---

## 10. Input subsystem policy

Input is implemented as: backend (GLFW) → event queue → physical state → action resolver → per-frame `InputFrame` snapshot → `Input` facade. It is **feature-frozen**. Allowed work: bug fixes and wiring that the game actually needs.

Known limitations tracked in `STATUS.md`:
- Input contexts are stubbed; the resolver ignores `active_contexts`. Wire them (bindings carry an optional context, invalid meaning "always active") only when a pause menu or ImGui capture needs them.
- Action-level `pressed` can miss a same-frame tap; fix before jump buffering.
- The first mouse move after start, focus regain or cursor-mode change produces a large delta; fix before mouse-look or drag.

---

## 11. 2D and 2.5D scope

**2D baseline:** window/runtime, input, 2D rendering, transforms, camera, textures, entities/scene, collision, simple animation/audio, level setup, packaging.

**2.5D extension:** depth, layers, parallax, selected 3D assets and lighting, driven by a representative game. The 2D path stays first-class.

---

## 12. Testing and quality gates

- Keep the reference game building and playable at each milestone.
- Test pure logic (input state/resolver/queue, later collision and lifecycle) in a small test executable.
- Build Debug and Release from a clean checkout (CI once M1b lands).
- Keep warnings enabled; fix meaningful ones.
- Test the packaged game from a clean directory before calling M5 done.

A milestone is complete when the reference game uses the feature successfully and important behavior is tested.

---

## 13. Decision register

| Topic | Current direction | Status |
|---|---|---|
| Product scope | 2D first; focused 2.5D later | Agreed |
| Main priority | Playable games early; engine grows alongside | **Primary rule** |
| Authoring workflow | C++ code-first; no mandatory editor | Agreed |
| Debug UI | ImGui for tuning/inspection; introduced at M1b | Agreed |
| Runtime | Engine owns loop and lifecycle (`Run` + `Application`) | **Implemented** |
| Game interface | `Engine::Game` callbacks; no `GameInstance` yet | **Implemented** |
| Gameplay structure | `Scene` first; `World`/`GameMode`/`GameInstance` deferred | Changed in 1.2 |
| Entity model | Actor/Component vs handles vs ECS undecided; decide after M1b | Open |
| Engine deployment | `engine` SHARED + game exe, for hot-reload potential | **Current decision** |
| Shared vs static revisit | Reconsider at end of M3 if hot reload unused | Proposed |
| CMake structure | One engine target, one game target | Agreed |
| Renderer | Direct OpenGL `Renderer2D`; no RHI | Agreed |
| Math library | glm, as a submodule | Pending (not yet added) |
| Input | Feature-frozen; wire contexts when needed | Agreed |
| First game | Small 2D platformer | Reference project |
| Physics backend | Own AABB collision in game code first | Deferred |
| Asset serialization / hot reload / plugins / editor | Not initial requirements | Deferred |
| Full 2.5D feature set | Driven by a representative game | Deferred |

---

## 14. How we change architecture safely

1. Inspect the actual repository and build graph before changing code.
2. Pick the smallest change that advances the current milestone.
3. Update code, CMake, tests, `STATUS.md` and this document together when a decision changes.
4. Mark decisions as agreed, proposed, or deferred.
5. Preserve a playable reference game during refactors.
6. Prefer a small, tested working slice over a large unfinished subsystem.

---

## Immediate next steps

1. Apply the small `Application::Run` fixes listed in `STATUS.md` (OnResize after OnStart, standard includes, backlog check, consistent shutdown rule).
2. Add glm as a submodule; fix `.gitignore`.
3. Implement `Renderer2D` (ortho camera, batched quads) and move viewport/clear into it.
4. Build the player and platforms in game code with intent sampling and a fixed step.
5. Add the ImGui tuning panel.
6. Only then decide the entity model.

**Guiding principle:** the engine is successful when it helps us make games, not when every planned subsystem exists.
