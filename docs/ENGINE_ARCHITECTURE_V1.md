# OpenGL Engine — Architecture Map & Game-First Roadmap

**Status:** Living architecture baseline  
**Version:** 1.3  
**Last updated:** 2026-10-23  
**Primary priority:** Make a real, playable 2D game as early as possible, then grow the engine in response to actual game needs.

> This document describes intended architecture and the development plan. It does not claim that every listed system exists. **Implementation status lives in [`STATUS.md`](STATUS.md).** Update this document only when an architecture decision changes.

### Changes in 1.3

- **M1 is complete:** renderer, camera, playable prototype, ImGui tuning panel, unit tests and CI all exist. Details are in `STATUS.md`.
- Renderer is named `Renderer` and the camera `Camera` (the earlier `Renderer2D`/`Camera2D` names are retired). World units are Y-up with a fixed vertical extent.
- Debug UI decision recorded: the engine owns the ImGui context; the game sees only a narrow `DebugUI` interface. ImGui is a private static dependency.
- Testing policy recorded: pure logic is tested in a separate `engine_tests` executable (doctest), and CI builds and tests Debug and Release.
- **New deferred items:** render interpolation alpha (known quivering at 120 Hz) and a platform-neutral debug UI backend (`Application` currently depends on GLFW and OpenGL ImGui backends directly).
- M2 is split into M2a (gameplay slice in game code) and M2b (entity model decision).

### Changes in 1.2

- `Game` + `Engine::Run(game, config)` replaced `GameInstance`/`GameMode` as the gameplay entry point.
- `GameInstance`, `World`, `GameMode` deferred; a plain `Scene` is preferred until a proven need.
- Actor/Component marked unproven; the model is chosen after the prototype.
- Shared library kept deliberately (future hot-reload), with the accepted costs listed (see 8).
- Input subsystem declared feature-frozen.

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
- Keep GLFW, OpenGL and ImGui out of game-facing headers.
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

A small side-scrolling platformer. Working title: **Shadow Runner** (executable target: `gamebox`).

Current (M1): window, engine-owned loop, keyboard input, a rectangle player with acceleration, friction, buffered jump, coyote time and variable jump height, AABB collision against a small level, live tuning panel.

Next: a larger level with camera follow, pickups, a hazard, a goal, one enemy. Later: sprites, animation, sound, multiple levels, save data, parallax, lighting, selected 3D objects.

### Rules for avoiding overengineering

1. Do not build an Actor/Component/World framework before the prototype shows which lifecycle and ownership features it needs.
2. Do introduce extension points early enough that game code never owns the loop.
3. Prefer a simple explicit C++ API over reflection, macros or hidden magic.
4. No asset database, prefab system, scene serializer, editor or plugin framework until a real workflow needs one.
5. If a feature can live in game code and is not reusable yet, keep it there. Promote it after a second use or a clear cross-cutting need.
6. Keep the reference game working after every milestone.
7. Freeze subsystems that are already "enough" (currently: input) rather than polishing them ahead of need.
8. Every deferred item gets a written trigger (see the register in `STATUS.md`).

---

## 3. Architecture map

The engine owns execution. The game supplies behavior through stable extension points.

```text
                    GAME EXECUTABLE (gamebox)
              (Shadow Runner / user's game)
                          |
              class PlatformerGame : Engine::Game
              int main() { return Engine::Run(game, config); }
                          |
                          v
+----------------------------------------------------------+
|                  ENGINE RUNTIME (DLL)                    |
|                                                          |
|  Engine::Run  -> log init/shutdown, exception reporting  |
|  Application  -> owns window, input, renderer, debug UI, |
|                  clock, frame loop                       |
|    - poll events -> Input::BeginFrame                    |
|    - OnUpdate(dt)  (once per rendered frame)             |
|    - OnFixedUpdate(step)  (0..N per frame)               |
|    - renderer clear -> OnRender -> OnDebugUI -> swap     |
|                                                          |
|  Services                                                |
|  - Window / Platform events        [implemented]         |
|  - Input (actions, contexts)       [implemented, frozen] |
|  - Renderer / Camera (batched quads)[implemented]        |
|  - Debug UI (ImGui, narrow API)    [implemented]         |
|  - Collision / Movement helpers    [game code first]     |
|  - Textures / Audio / Animation    [when needed]         |
+----------------------------------------------------------+
                          |
                          v
            Platform + graphics backends
             GLFW / OpenGL / OS facilities
```

This is a logical architecture, not a promise that each box becomes a separate target. Keep one `engine` target and one game target until there is a concrete dependency, testing or build reason to split.

### Dependency direction

```text
Game code
   -> Public engine API (Game, Application, Input, Renderer, Camera, DebugUI ...)
      -> Private implementations (src/)
         -> GLFW / OpenGL / ImGui / OS / third-party libraries
```

Game code must not call GLFW, touch OpenGL buffers, call ImGui directly, or own the runtime loop.

---

## 4. Core concepts

### `Engine::Run` and `Application` (implemented)

`Run` initialises logging, constructs `Application`, converts exceptions to a log entry and exit code, and shuts logging down last. `Application` owns the window, input system, renderer, optional debug UI layer and clock, and runs the loop. Destruction order: debug UI layer, renderer (needs the GL context), input, window, then the logger. Nothing throws across the DLL boundary.

### `Game` (implemented)

An abstract class with overridable callbacks: `OnStart`, `OnUpdate`, `OnFixedUpdate`, `OnRender`, `OnResize`, `OnDebugUI(DebugUI&)`, `OnShutdown`. The game registers input actions in `OnStart`; `Application` finalizes the action map afterward. The game never owns the loop.

Contract notes:
- `OnUpdate` samples input intent once per rendered frame.
- `OnFixedUpdate` advances simulation at a constant step and must not read raw per-frame "pressed" input directly; it consumes intent captured in `OnUpdate`.
- `OnResize` is delivered after `OnStart` and never with a zero size.
- `OnDebugUI` is called only when `AppConfig::enable_debug_ui` is true, between the engine's ImGui begin and end.

### `Renderer` and `Camera` (implemented)

- **Style:** immediate-mode batch renderer. The game submits colored quads each frame; the renderer batches them into one dynamic vertex buffer with a static index buffer and one embedded shader, flushing when full or at `EndScene`.
- **Ownership:** `Application` owns the renderer; the game reaches it through `App().GetRenderer()`. The public header is a pimpl facade; no GL types leave `src/`.
- **Frame split:** the engine owns clear, viewport and buffer swap (frame functions are private to `Application`). The game owns scene submission through `BeginScene(camera)`, `DrawQuad(center, size, color)` and `EndScene()`.
- **Coordinates:** world units, Y-up. `Camera` has a fixed vertical extent; the width follows the framebuffer aspect ratio. Game code never depends on window resolution.
- **State:** depth test off, alpha blending on, draw order equals submission order. Draw-call and quad counts are tracked each frame.
- **Robustness:** GL objects use small RAII wrappers; shader compile or link failure throws during init and is reported by `Engine::Run`; minimized windows never set a zero viewport.
- **Not here yet:** textures, rotation, layers, an RHI. Each waits for a real user.

### `DebugUI` (implemented)

The engine owns the ImGui context and the GLFW/OpenGL3 backends (`ImGuiLayer`, internal). The game receives a narrow `DebugUI&` with only the widgets it needs (currently `Begin`, `End`, `Text`, `Separator`, `Button`, `Checkbox`, `SliderFloat`). ImGui is a private static dependency of the engine: its headers and symbols are not exported, which also keeps a future hot-reloaded game DLL from needing to share ImGui's context. New widgets are added only on real need.

Known coupling (deferred): `Application` and `ImGuiLayer` currently depend on GLFW and OpenGL directly. See section 10.

### `Scene` (planned, M2b)

A simple container for the current level's entities and level-scoped state, built in C++. It replaces `World`/`GameMode` until a proven need arises. Level loading is a function that fills a scene.

### Entity model (undecided, M2b)

Unreal's `Actor*` is safe because of UObject, garbage collection and reflection. We have deliberately excluded those, so **object lifetime is our real risk**. Decide after the M2a gameplay slice between:

1. Plain structs in game-owned containers, with stable IDs or handles.
2. A small custom handle-based entity store.
3. A small ECS library (for example EnTT).

Whichever is chosen must support safe spawn/destroy during update (deferred destruction) and must not expose raw pointers that can dangle.

### `GameInstance`, `GameMode` (deferred)

In Unreal these exist largely for networking, server authority and level travel. A single-player 2D game does not need them. Reintroduce `GameInstance` only if persistent cross-scene state becomes awkward to hold in `Game`.

---

## 5. Game-facing API (current)

```cpp
class PlatformerGame final : public Engine::Game
{
public:
    bool OnStart() override;                       // register actions, build level
    void OnUpdate(float dt) override;              // sample input intent
    void OnFixedUpdate(float step) override;       // movement, collision
    void OnRender() override;                      // draw via Renderer
    void OnResize(int w, int h) override;          // camera aspect
    void OnDebugUI(Engine::DebugUI& ui) override;  // tuning panel
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
OnUpdate:      read actions -> fill Intent{ move_x, jump_held, jump_pressed (latched) }
OnFixedUpdate: consume Intent -> integrate velocity -> resolve collisions
OnRender:      draw using current positions (interpolate once the alpha is added)
```

---

## 6. Game-first development roadmap

### Milestone 0 — Reliable build, entry point, loop (done)

Engine DLL and game executable build cleanly; window, input, logging; `Game`, `Application`, `Engine::Run`, frame timing, fixed-step accumulator; minimize and resize handling; clean shutdown order.

### Milestone 1a — First pixels (done)

glm as a submodule; `Renderer` with an ortho `Camera`, batched colored quads, embedded shaders; viewport and clear owned by the renderer.

### Milestone 1b — First playable prototype (done)

Player controller and platform collision in game code; fixed-step movement with input-intent sampling, jump buffering and coyote time; ImGui tuning panel; unit tests and CI (Debug and Release).

**Exit criteria met:** a controllable character runs and jumps across platforms; movement is framerate independent; tests and CI are green. Known issue: quivering at 120 Hz (interpolation deferred).

### Milestone 2a — Gameplay slice in game code (next)

- A level larger than the screen with camera follow.
- A pickup, a hazard, a goal, and one patrolling enemy, as plain structs in vectors.
- Restart path covering all of the above.

**Purpose:** force real spawn/destroy, several moving bodies and a second user of the collision code. Do not add `Scene` or an entity framework yet.

### Milestone 2b — Minimum framework the slice needs

- Choose the entity model (see 4) from the actual pain in M2a.
- `Scene` and deferred spawn/destroy only if M2a showed the need.
- Promote AABB collision into the engine (the enemy is the second user).
- Unit tests for lifecycle and core logic.

**Exit criteria:** player, platforms, pickup, hazard and enemy live in the chosen structure; lifetime tests pass; the game still plays.

### Milestone 3 — Small complete 2D game

- Textures/sprites (stb_image) and an asset-path strategy.
- Animation and audio as required.
- Level transition, minimal UI.
- Render interpolation alpha if not done earlier.

**Exit criteria:** someone other than the author can run and finish the game from a clean build. At the end of M3, revisit SHARED vs STATIC.

### Milestone 4 — Workflow improvements driven by friction

- Inspector and collision/transform overlays.
- Frame-time and renderer statistics beyond the current counters.
- Input diagnostics.
- Expanded tests (CI already exists).

### Milestone 5 — Production basics

- Debug and Release configurations, packaging of exe + `engine.dll` + third-party DLLs + assets.
- Debug UI off by default in Release.
- Missing-resource handling and a clean-checkout build/run guide.

### Milestone 6 — Focused 2.5D features (only if a game needs them)

Depth/layer policy, parallax, perspective camera, selected 3D props, lighting.

### Milestone 7 — Grow by demand

Save/load, data-driven templates, tilemap tooling, controller support, remappable bindings, hot reload (if still wanted), more physics, platform and debug UI backend abstraction (if a second platform is wanted).

---

## 7. Feature policy

| Priority | Features | Why |
|---|---|---|
| **Done** | Window, input, logging, `Game`/`Application` loop, fixed-step timing, `Renderer`/`Camera`, `DebugUI`, player and collision in game code, tests, CI | Foundation and first playable prototype exist |
| **Now** | Larger level, camera follow, pickup, hazard, goal, enemy (game code) | Reveals the entity-model needs |
| **Soon** | Entity model decision, `Scene` if needed, collision promoted to the engine | Driven by the gameplay slice |
| **When the game needs it** | Textures, animation, audio, level transitions, resource caching, render interpolation alpha | Add when they improve the next playable milestone |
| **Later** | 2.5D lighting, 3D props, perspective, advanced physics, save/load, serialization, hot reload, platform abstraction for debug UI | Avoid delaying the first game |
| **Not planned** | Full editor, visual scripting, reflection/GC, plugin ecosystem, RHI abstraction | Outside the scope unless a strong need emerges |

---

## 8. Shared-library and CMake policy

**Decision:** keep `engine` as a `SHARED` library plus a game executable (`gamebox`). The primary reason is keeping a future hot-reload path open.

### Costs we are consciously accepting

- Export annotations on every public class/function (`ENGINE_API`), and discipline about STL/third-party types in public headers.
- Static spdlog gets linked into both exe and DLL, which works only while they share the same CRT. A hot-reloaded game DLL would add a third copy; solve that with an exported logging function if and when we reach it.
- Unit tests cannot link private classes through the DLL. Pure logic is tested by compiling those `.cpp` files directly into `engine_tests`.
- Binary compatibility across compiler versions is not guaranteed; the API is source-level.

### Revisit trigger

If hot reload is not attempted by the end of Milestone 3, reconsider switching the default to `STATIC` with `SHARED` as a CMake option (`ENGINE_API` gains a third branch that expands to nothing). Unreal itself is modular in the editor and monolithic static in shipping builds.

### DLL requirements

- Single `ENGINE_API` macro: export when building the engine, import for consumers, visibility attribute on GCC/Clang, empty otherwise.
- `CXX_VISIBILITY_PRESET hidden` and `VISIBILITY_INLINES_HIDDEN ON` so Linux builds catch missing exports.
- Small data-only structs read by the game either carry `ENGINE_API` or are inline in the header.
- Keep implementation-only dependencies PRIVATE when public headers do not need them (GLFW, glad, ImGui). glm and spdlog are PUBLIC because public headers use them.
- Nothing may throw across the DLL boundary; `Engine::Run` is the catch-all.
- Hot-reload shape (not implemented): a game DLL would export one `extern "C"` factory returning `Game*`.

### Include and target boundaries

Public headers live under `engine/include/engine/`. Implementation lives under `engine/src/`. Use target-scoped CMake settings only. Third-party includes are `SYSTEM` so warnings stay readable. Warnings are on (`/W4` or `-Wall -Wextra`) for the engine, the game and the tests.

### Dependencies

glfw, spdlog, glm, imgui and doctest are git submodules (ImGui and doctest pinned to release tags, not `master`). glad and other header-only libraries live under `external/include`. ImGui is built as a static library and linked PRIVATE to the engine. doctest is test-only.

### Presets and CI

`CMakePresets.json` has configure, build and test presets for x64 Debug and Release. Local builds and CI use the same presets. CI runs on GitHub Actions (Windows) for Debug and Release: configure, build, test.

---

## 9. Runtime frame/lifecycle (implemented)

1. `Run`: init logging, construct `Application`.
2. Create window, input and renderer (needs the GL context), plus the debug UI layer if enabled; set the resize callback; set the viewport.
3. `Game::OnStart`; finalize the input action map; deliver the first `OnResize`.
4. Each frame: compute dt (clamped to `max_frame_time`), poll events, `Input::BeginFrame`.
5. If minimized: drain input, sleep briefly, reset the clock, skip game and render.
6. `OnUpdate(dt)`.
7. Fixed-step loop: `OnFixedUpdate(step)` up to `max_fixed_steps_per_frame`; drop any remaining backlog.
8. Renderer clears; `OnRender`; if the debug UI is enabled: ImGui begin, `OnDebugUI`, ImGui render; swap; `Input::EndFrame`; update FPS.
9. On exit: `OnShutdown`, destroy debug UI layer, renderer, input, window in that order, then shut down logging.

Lifecycle contract points still to define when scenes arrive: spawn/destroy during update, scene transitions, shutdown order of scene objects. The interpolation alpha is added to `FrameTime` when it is scheduled (see the register).

---

## 10. Subsystem policies

### Input (feature-frozen)

Backend (GLFW) → event queue → physical state → action resolver → per-frame `InputFrame` snapshot → `Input` facade. Allowed work: bug fixes and wiring that the game actually needs. Known limitations and their triggers are in `STATUS.md`:
- Input contexts are stubbed; the resolver ignores `active_contexts`. Wire them (bindings carry an optional context, invalid meaning "always active") when a pause menu or ImGui capture needs them. A skipped test marks the gap.
- The first mouse move after a cursor-mode change can produce a large delta; fix before mouse-look.

### Rendering and interpolation

The fixed step is 60 Hz and rendering uses current positions. On high-refresh displays this shows as quivering (observed at 120 Hz). The planned fix, **deferred**: add `alpha = accumulator / fixed_step` to `FrameTime`; the game keeps previous and current simulation state and interpolates in `OnRender`. Trigger: movement polish, a high-refresh playtest, or sprites making the jitter more visible.

### Debug UI and platform coupling

`Application` currently includes the GLFW window header, casts the window to `GLFW_Window` to get the native handle, and `ImGuiLayer` uses the GLFW and OpenGL3 ImGui backends. That is acceptable with one platform and one graphics API. **Deferred:** a platform-neutral debug UI backend. The likely shape follows the input backend pattern: the window (or a platform factory) creates a debug UI backend, and `Application` talks only to an interface. Trigger: a second window/platform backend (for example SDL), a second graphics backend, or `Application` needing to build without GLFW headers. Until then, do not add the abstraction.

---

## 11. 2D and 2.5D scope

**2D baseline:** window/runtime, input, 2D rendering, transforms, camera, textures, entities/scene, collision, simple animation/audio, level setup, packaging.

**2.5D extension:** depth, layers, parallax, selected 3D assets and lighting, driven by a representative game. The 2D path stays first-class.

---

## 12. Testing and quality gates

- Keep the reference game building and playable at each milestone.
- Pure logic (input queue, physical state, action resolver and map, collision, player movement) is tested in `engine_tests` using doctest, compiled directly from source with no GLFW, GL or DLL dependency. Internal headers are not made public for tests.
- Expectations come from the code's real contracts and from `Tuning` values, not hard-coded numbers, so retuning does not break tests. Do not weaken an assertion just to get a green run.
- Known gaps are kept visible with skipped tests (currently: context filtering).
- Build and test Debug and Release locally and in CI before closing a milestone.
- Keep warnings enabled; fix meaningful ones.
- Test the packaged game from a clean directory before calling M5 done.
- Code that needs a GL context (renderer, window, `Application`) is verified manually for now (resize, minimize, broken shader, shutdown); a headless GL test setup is deferred.

A milestone is complete when the reference game uses the feature successfully, important behavior is tested, and CI is green.

---

## 13. Decision register

| Topic | Current direction | Status |
|---|---|---|
| Product scope | 2D first; focused 2.5D later | Agreed |
| Main priority | Playable games early; engine grows alongside | **Primary rule** |
| Authoring workflow | C++ code-first; no mandatory editor | Agreed |
| Runtime | Engine owns loop and lifecycle (`Run` + `Application`) | **Implemented** |
| Game interface | `Engine::Game` callbacks; no `GameInstance` yet | **Implemented** |
| Gameplay structure | `Scene` first; `World`/`GameMode`/`GameInstance` deferred | Agreed |
| Entity model | Actor/Component vs handles vs ECS undecided; decide after M2a | Open |
| Engine deployment | `engine` SHARED + game exe, for hot-reload potential | **Current decision** |
| Shared vs static revisit | Reconsider at end of M3 if hot reload unused | Proposed |
| CMake structure | One engine target, one game target, one test target | Agreed |
| Renderer | Direct OpenGL batched `Renderer`, pimpl facade; no RHI | **Implemented** |
| Coordinates and camera | Y-up world units, fixed vertical extent, aspect from framebuffer | **Implemented** |
| Frame ownership | Engine owns clear/viewport/swap; game owns scene submission | **Implemented** |
| Math library | glm, submodule, PUBLIC | **Implemented** |
| Debug UI | Engine-owned ImGui; narrow `DebugUI` interface; ImGui private static dependency | **Implemented** |
| Debug UI platform abstraction | Backend behind an interface, like input | Deferred (trigger: second platform or graphics backend) |
| Input | Feature-frozen; wire contexts when needed | Agreed |
| Interpolation | Alpha in `FrameTime`, game-side lerp | Deferred (known issue at 120 Hz) |
| Collision | AABB in game code; promote on second user (M2a enemy) | Agreed |
| Testing | doctest, separate `engine_tests`, pure logic only | **Implemented** |
| CI | GitHub Actions, Windows, Debug and Release, via presets | **Implemented** |
| First game | Small 2D platformer | Reference project |
| Physics backend | Own AABB collision in game code first | Deferred |
| Asset serialization / hot reload / plugins / editor | Not initial requirements | Deferred |
| Full 2.5D feature set | Driven by a representative game | Deferred |

---

## 14. How we change architecture safely

1. Inspect the actual repository and build graph before changing code.
2. Pick the smallest change that advances the current milestone.
3. Update code, CMake, tests, `STATUS.md` and this document together when a decision changes.
4. Mark decisions as agreed, proposed, or deferred, and give every deferral a trigger.
5. Preserve a playable reference game during refactors.
6. Prefer a small, tested working slice over a large unfinished subsystem.

---

## Immediate next steps

1. M2a: larger level with camera follow, then pickup, hazard, goal and one patrolling enemy in game code.
2. M2b: decide the entity model from what M2a made painful; promote collision into the engine.
4. M3: sprites and an asset-path strategy; schedule the interpolation alpha.

**Guiding principle:** the engine is successful when it helps us make games, not when every planned subsystem exists.
