# Implementation Status

**Last updated:** 2026-10-23  
**Companion to:** [`ENGINE_ARCHITECTURE_V1.md`](ENGINE_ARCHITECTURE_V1.md) (intent). This file records what actually exists.

Legend: ✅ done · 🟡 partial / has known issues · ⬜ not started · ⏸ deferred on purpose (see the Deferred register)

> Status is based on the last synced source and the author's reported results (local Debug/Release runs, test output, CI). Items marked *(unverified)* could not be confirmed from the synced files.

---

## Current milestone

**M0 (build, loop, entry point): done.**  
**M1a (renderer, first pixels): done.**  
**M1b (playable prototype): done.** A controllable character runs and jumps across a small level; gravity, jump, friction and timing are tunable live through the debug panel.  
**Tests and CI: done.** 51 test cases pass (1 skipped on purpose) in Debug and Release; CI is green.  
**Next: M2a, a gameplay slice in game code** (see Next steps).

Current behavior: a platformer test level in a window titled `Shadow Runner | NN FPS`. A/D move, Space jumps (buffered, with coyote time and variable height), R resets, Esc quits. An ImGui "Tuning" panel edits the `Tuning` struct and shows FPS, draw calls, quad count and player state.

### M1 verification record

| Check | Result |
|---|---|
| Jump height with vsync on and off | Passed; jump height looks correct |
| Window resize and viewport correction | Passed |
| Minimize and restore | Passed |
| Broken shader handling | Passed: critical log, exit code 1, no crash |
| Movement at 120 Hz | **Known issue:** visible quivering (60 Hz fixed step, no interpolation). Deferred, see the register |
| Unit tests, Debug | 51 cases passed, 0 failed, 1 skipped; 29115 assertions (doctest 2.4.11) |
| Unit tests, Release | Passed |
| CI (Windows, x64-debug and x64-release) | Green |

---

## Subsystems

### Build and project

| Item | State | Notes |
|---|---|---|
| Root CMake (C++20, subdirectories) | ✅ | |
| `engine` SHARED target + `Engine::Engine` alias | ✅ | Kept SHARED on purpose (hot-reload potential) |
| `gamebox` executable | ✅ | Links only `Engine::Engine`; no ImGui or GL headers reach it |
| `ENGINE_API` export macro | ✅ | `InputFrame` exported via `ENGINE_API` |
| Warnings (`/W4`, `-Wall -Wextra`) on engine, game and tests | ✅ | |
| Hidden visibility preset | ✅ | |
| `SPDLOG_ACTIVE_LEVEL` public definition | ✅ | TRACE in Debug, INFO otherwise |
| glm | ✅ | Submodule, `glm::glm` linked PUBLIC |
| ImGui | ✅ | Submodule, static library, linked PRIVATE to the engine |
| doctest | ✅ | Submodule, pinned to v2.4.11, test-only |
| `header-only-libraries` marked SYSTEM | ✅ | glad and similar; linked PRIVATE to the engine |
| Precompiled header (`pch.h`) | ✅ | Engine only; `#pragma once` present |
| CMake presets: configure, build and test (x64 Debug/Release) | ✅ | CI uses the same presets |
| `.gitignore` for `logs/`, `CMakeUserPresets.json`, workflows exception | ✅ | |
| README build and test instructions | ⬜ | `git clone --recurse-submodules`, preset, run, `ctest --preset` |
| CMake presets cleanup (Linux/macOS are VS defaults) | ⬜ | See Deferred |

### Platform and window

| Item | State | Notes |
|---|---|---|
| GLFW window, GL 3.3 core, glad loader | ✅ | |
| Window owns GLFW callbacks and user pointer | ✅ | Forwards to the input backend |
| Input backend / window detach safety | ✅ | Backend destructor only detaches; either can be destroyed first |
| Framebuffer resize callback | ✅ | |
| `RequestClose`, `IsMinimized` | ✅ | |
| Resizable windows | 🟡 | Loop blocks during edge-drag on Windows (accepted) |
| Platform independence of `Application` | 🟡 | `Application` includes the GLFW window header and casts to it for ImGui. Deferred, see the register |
| macOS forward-compat hint | ⏸ | Not a target |
| Multiple windows | ⏸ | `glfwTerminate` in the window destructor |

### Runtime

| Item | State | Notes |
|---|---|---|
| `Engine::Run` (log init/shutdown, exception catch-all) | ✅ | No exceptions cross the DLL |
| `Application` (owns window, input, renderer, debug UI layer, clock) | ✅ | Destruction order: ImGui layer, renderer, input, window, then logger |
| `Game` interface | ✅ | `OnStart/OnUpdate/OnFixedUpdate/OnRender/OnResize/OnDebugUI/OnShutdown` |
| Frame time, clamped dt, fixed-step accumulator | ✅ | |
| Fixed-step backlog check | ✅ | |
| `OnResize` delivered after `OnStart` | ✅ | Viewport is set early by the engine |
| `OnShutdown` rule | ✅ | Called only if `OnStart` returned true, on every path |
| FPS in title | ✅ | |
| Minimized handling | ✅ | |
| Render interpolation alpha | ⏸ | Known issue at 120 Hz; see the register |
| Exception thrown from `OnStart` | 🟡 | Caught by `Engine::Run`; `OnShutdown` not called (acceptable under the rule) |

### Renderer and camera

| Item | State | Notes |
|---|---|---|
| `Renderer` (pimpl facade, `Application` owns it) | ✅ | Direct OpenGL; no RHI |
| Batched colored quads, dynamic VBO, static IBO, embedded shader | ✅ | Max 10,000 quads per batch, flushed automatically |
| Y-up world units, ortho `Camera`, fixed vertical extent | ✅ | Aspect from the framebuffer |
| Engine owns clear, viewport and swap; game owns `BeginScene/DrawQuad/EndScene` | ✅ | Frame functions are private to `Application` |
| Depth test disabled, submission order preserved | ✅ | |
| Draw-call and quad stats | ✅ | Reset each frame |
| GL object cleanup (RAII wrappers) | ✅ | |
| Shader compile/link failure handling | ✅ | Verified |
| Textures / sprites | ⬜ | M3 |
| Rotation, layers / z-order | ⏸ | When a game needs them |

### Debug UI

| Item | State | Notes |
|---|---|---|
| Engine-owned ImGui context and GLFW/OpenGL3 backends | ✅ | `ImGuiLayer`, internal |
| Narrow `DebugUI` interface for the game | ✅ | `Begin/End/Text/Separator/Button/Checkbox/SliderFloat` |
| `Game::OnDebugUI(DebugUI&)` callback | ✅ | Called between ImGui begin and end each frame |
| Tuning panel (all `Tuning` fields, FPS, renderer stats, player state, reset button) | ✅ | |
| Platform-neutral debug UI backend | ⏸ | See the register |
| Keyboard/mouse capture feeding input contexts | ⏸ | Space still jumps while a slider has focus |

### Input

Policy: **feature-frozen**. Bug fixes only until the game demands more.

| Item | State | Notes |
|---|---|---|
| GLFW input backend, event queue | ✅ | |
| Physical key/mouse state with pressed/released | ✅ | |
| Raw input works before action finalization | ✅ | |
| Action map (digital, scalar, vector2), bindings | ✅ | |
| Action resolver | ✅ | Sub-frame taps preserved |
| `InputFrame` snapshot, `Input` facade | ✅ | |
| Event queue overflow warning | ✅ | Oldest event is dropped on overflow |
| Mouse delta spike on first move / focus change | ✅ | |
| Mouse delta spike on cursor-mode change | ⏸ | Fix before mouse-look |
| Input contexts | 🟡 | Stack exists; `Resolve()` ignores `active_contexts` (C4100 accepted) |
| Consumption policy | ⬜ | Never read; real use is ImGui capture |

**Known input limitations (remaining)**

1. Cursor-mode change can still create a large mouse delta.
2. `ModifierKey::CapsLock/NumLock` can never be set (modifiers derive from key state).
3. `Disable` on a context removes it from the stack so `Enable` cannot restore its order.
4. Raw `WasKeyPressed` per rendered frame is unsafe inside a fixed step; the game uses the intent pattern.
5. On queue overflow the oldest event is dropped, which could be a KeyUp (very unlikely at 4096 events).
6. Scalar actions clamp after each binding is added, so stacked bindings with mixed signs can depend on order. Irrelevant for the current A/D setup.

### Game (`gamebox`)

| Item | State | Notes |
|---|---|---|
| `PlatformerGame` (own files) | ✅ | |
| Input intent struct sampled in `OnUpdate`, consumed in `OnFixedUpdate` | ✅ | `MoveX` scalar and `Jump` digital actions |
| Player state and `StepPlayer` | ✅ | Acceleration, friction, jump buffer, coyote time, variable jump height, fall cap |
| AABB collision, axis-separated | ✅ | Kept in game code; promote on a second user |
| Level data (`BuildLevel`) | ✅ | Hardcoded rectangles; fits one screen |
| `Tuning` struct, live-editable | ✅ | |
| Reset on key, kill-plane and world-bound reset | ✅ | |
| Camera follow, larger level | ⬜ | M2a |
| Pickup, hazard, goal, enemy | ⬜ | M2a |
| Entity model, `Scene` | ⬜ | M2b, decided from M2a pain |

### Tests and CI

| Item | State | Notes |
|---|---|---|
| `engine_tests` executable (no GL, no window, no DLL link) | ✅ | Compiles the tested `.cpp` files directly |
| InputEventQueue tests | ✅ | |
| PhysicalInputState tests | ✅ | |
| InputActionResolver and InputActionMap tests | ✅ | One test skipped on purpose (context filtering) |
| Collision tests | ✅ | |
| Player tests | ✅ | Includes jump height and step-size independence |
| Totals | ✅ | 51 passed, 0 failed, 1 skipped; 29115 assertions |
| Debug and Release runs | ✅ | |
| GitHub Actions CI (Windows, Debug and Release) | ✅ | Green |
| Tests for renderer, window, `Application` | ⏸ | Need a GL context; see the register |

### Logging

| Item | State | Notes |
|---|---|---|
| spdlog engine and game loggers, console + file sinks | ✅ | Files go to `logs/` |
| `LOG_TRACE`/`LOG_DEBUG` active in Debug | ✅ | |
| One spdlog copy across exe and DLL | ⏸ | Matters only for a hot-reloaded game DLL |

### Not started

Textures and sprites, camera follow, animation, audio, scene and entity model, level files, packaging, README.

---

## Open issues, prioritized

**Do now (small)**
1. README: clone with submodules, configure/build/run presets, run tests.

**Known issue (deferred, tracked)**
1. Quivering at 120 Hz (no render interpolation).

**Housekeeping**
1. Trim CMake presets (Linux/macOS are VS defaults).
2. Update this file after every milestone.

---

## Next steps (in order)

1. **M2a, gameplay slice in game code:** a level larger than the screen with camera follow; then a pickup, a hazard, a goal and one patrolling enemy, all as plain structs in vectors. The enemy needs the same collision code, which is the second user that justifies promoting AABB and collision into the engine.
2. **M2b:** choose the entity model from what hurt in M2a (plain structs with IDs, handles, or an ECS). Add `Scene` only if M2a shows a need.
3. **M3:** textures and sprites with an asset-path strategy, animation, audio, level transition, minimal UI. At the end of M3, revisit SHARED vs STATIC.
4. Render interpolation alpha when movement polish starts, or earlier if the 120 Hz quivering becomes a problem.

---

## Deferred register

Deferred on purpose. Each item has a trigger: do not start it before the trigger is true.

| Item | Why deferred | Trigger to revisit |
|---|---|---|
| **Render interpolation alpha** (`alpha = accumulator / fixed_step` in `FrameTime`; the game keeps previous and current position and lerps in `OnRender`) | Known quivering at 120 Hz with a 60 Hz fixed step; acceptable while the engine is tiny | Movement polish, a playtest on a high-refresh monitor, or before M3 sprites make the jitter more visible |
| **Platform-neutral debug UI / ImGui abstraction** (move the platform and graphics specific ImGui backend setup out of `Application`) | `Application` currently includes the GLFW window header, casts the window to `GLFW_Window`, and `ImGuiLayer` takes a `GLFWwindow*` and uses the OpenGL3 backend. Fine for one platform; a second one would force edits in `Application` | A second window or platform backend (for example SDL), a second graphics backend, or `Application` needing to compile without GLFW headers. Likely shape: the window (or a platform factory) creates a debug UI backend the same way it already creates the input backend, and `Application` only talks to an interface |
| Input contexts wired into `Resolve()` (optional context on bindings, invalid = always active) | No pause menu or UI yet | Pause menu or ImGui capture is needed |
| Input consumption policy | Never read | Same as above (ImGui `WantCaptureKeyboard/Mouse`) |
| Context `Disable`/`Enable` order restore | Depends on contexts being used | Same as above |
| Skipped test: bindings filtered by active context | Feature not implemented; the skip keeps the gap visible | Unskip when contexts are wired into `Resolve()` |
| Cursor-mode delta reset | Not needed for a platformer | Mouse-look, drag, or camera pan |
| `CapsLock`/`NumLock` modifiers | No use | A binding needs them |
| Controller / gamepad, remapping, text input handling | Not required for the first game | Playtesters need a gamepad, or a settings menu |
| Debug UI widgets beyond the current seven | The narrow interface grows only on real need | A debugging task needs `DragFloat`, `SameLine`, `Combo` and so on |
| Debug UI off by default in Release | Dev tool only | Packaging (M5) |
| Resizable windows and edge-drag freeze fix | Loop blocks during Windows edge-drag | The game needs a resizable window |
| Multiple windows (`glfwTerminate` in the window destructor) | One window only | A second window or tool window is needed |
| macOS/Linux support (`GLFW_OPENGL_FORWARD_COMPAT`, presets, Linux CI job) | Windows only for now | A non-Windows build is wanted |
| Pin GitHub Actions to commit SHAs, cache submodules and build | Hardening and speed | CI gets slow or the repo goes public |
| Tests that need a GL context (renderer, window, `Application`) | Unit tests run without a window by design | A renderer bug class that logic tests cannot catch, or a headless GL setup in CI |
| One spdlog copy across exe and DLL (exported logging function) | Only matters with a game DLL | Hot reload is attempted |
| Hot reload (`extern "C"` factory returning `Game*`) | Boundary is kept open, not implemented | Iteration time becomes painful; revisit at end of M3 |
| SHARED vs STATIC default | SHARED chosen for hot reload | End of M3 if hot reload is still unused |
| Exception from `OnStart` calling `OnShutdown` | Rare | `OnStart` starts doing fallible work (asset loading) |
| `GameInstance`, `World`, `GameMode` | Exist in Unreal for networking and level travel | Cross-scene state gets awkward to hold in `Game` |
| Entity model (plain structs + IDs, handles, or ECS such as EnTT) | Unproven; decide from real pain | After M2a |
| `Scene` and deferred spawn/destroy | Needs the entity model | M2b |
| AABB collision moved into the engine | Keep in game code until a second use | The patrolling enemy in M2a |
| Physics backend | Own AABB first | AABB stops being enough (slopes, many bodies) |
| Rotation and layers in `Renderer` | No user yet | A sprite or effect needs them |
| Textures / sprites / asset-path strategy | Colored quads first | M3 |
| Audio, animation, level transitions | Not needed for the prototype | When they improve the next playable milestone |
| RHI abstraction | One backend | A second graphics backend is planned |
| Editor, visual scripting, reflection/GC, plugin system | Non-goals | A concrete workflow demands one |
| Asset database, prefabs, scene serialization, save/load | No workflow needs them | After M3 |
| 2.5D features (depth, parallax, perspective, lighting) | 2D first | A representative game needs them (M6) |
| Packaging exe + DLLs + assets | Too early | M5 |

---

## Change log

| Date | Change |
|---|---|
| 2026-10-20 | P0 hardening applied (visibility, spdlog level, event pump, window-owned callbacks, main cleanup). |
| 2026-10-20 | P1 applied: `Game`, `Application`, `Engine::Run`, frame time, fixed step, resize/minimize handling. |
| 2026-10-20 | Review fixes: `OnResize` order, backlog check, includes, shutdown rule, glm submodule, game warnings, input sub-frame taps and mouse-delta spike. |
| 2026-10-22 | M1a: `Renderer` and `Camera`, engine-owned frame lifecycle. M1b: player, collision, level, tuning. ImGui integrated behind the narrow `DebugUI` interface. |
| 2026-10-23 | Tests (doctest 2.4.11, 51 cases, 1 skipped) and CI (Debug and Release) added and green. M1 declared complete. Deferred: render interpolation alpha, platform-neutral debug UI backend. `copilot-instructions.md` untracked. |
