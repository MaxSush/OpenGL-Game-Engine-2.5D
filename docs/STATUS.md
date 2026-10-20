# Implementation Status

**Last updated:** 2026-10-20  
**Companion to:** [`ENGINE_ARCHITECTURE_V1.md`](ENGINE_ARCHITECTURE_V1.md) (intent). This file records what actually exists.

Legend: ✅ done · 🟡 partial / has known issues · ⬜ not started · ⏸ deferred on purpose (see the Deferred register)

> Status is based on the last synced source (read, not compiled by the reviewer) and a successful build/run reported by the author. Items marked *(unverified)* could not be confirmed from the synced files.

---

## Current milestone

**M0 (reliable build, loop, entry point): done.**  
**Next: M1a (Renderer2D, first pixels).** The glm prerequisite is now met.

Current behavior: a dark grey window titled `Shadow Runner | NN FPS`. Esc quits. Space logs "Jump pressed!". Minimizing pauses update and render.

---

## Subsystems

### Build and project

| Item | State | Notes |
|---|---|---|
| Root CMake (C++20, subdirectories) | ✅ | |
| `engine` SHARED target + `Engine::Engine` alias | ✅ | Kept SHARED on purpose (hot-reload potential) |
| `ENGINE_API` export macro | ✅ | `InputFrame` exported via `ENGINE_API` |
| Warnings (`/W4`, `-Wall -Wextra`) on engine | ✅ | |
| Warnings on `game` target | ✅ | Expect a new `C4100` for the unused `dt` in `PlatformerGame::OnUpdate`; comment out the name |
| Hidden visibility preset | ✅ | |
| `SPDLOG_ACTIVE_LEVEL` public definition | ✅ | TRACE in Debug, INFO otherwise |
| `header-only-libraries` marked SYSTEM | ✅ | Linked PRIVATE to engine; not visible to the game, by design |
| glm dependency | ✅ | Submodule in `.gitmodules`, `add_subdirectory(glm)`, `glm::glm` linked PUBLIC. `.gitignore` no longer excludes it |
| glm cleanup: old `external/include/glm` copy removed | ✅ | Deleted it, so there is one glm only |
| `.gitignore` for `logs/`, `CMakeUserPresets.json` | ✅ | |
| `pch.h` has `#pragma once` | ✅ | `pch.cpp` no longer appears in the synced files |
| Standard includes in `application.cpp` | ✅ | Explicit, no longer PCH-dependent |
| README build instructions | ⬜ | `git clone --recurse-submodules`, preset, run |
| CMake presets cleanup (Linux/macOS are VS defaults) | ⬜ | Still present; see Deferred |
| Tests / CI | ⬜ | |

### Platform and window

| Item | State | Notes |
|---|---|---|
| GLFW window, GL 3.3 core, glad loader | ✅ | |
| Window owns GLFW callbacks and user pointer | ✅ | Forwards to input backend |
| Input backend / window detach safety | ✅ | Backend destructor only detaches; either can be destroyed first |
| Framebuffer resize callback | ✅ | |
| `RequestClose`, `IsMinimized` | ✅ | |
| Resizable windows | 🟡 | `resizable = false` by default; loop blocks during edge-drag on Windows (accepted) |
| macOS forward-compat hint | ⏸ | Not a target |
| Multiple windows | ⏸ | `glfwTerminate` in the window destructor |

### Runtime

| Item | State | Notes |
|---|---|---|
| `Engine::Run` (log init/shutdown, exception catch-all) | ✅ | No exceptions cross the DLL |
| `Application` (owns window, input, clock) | ✅ | Destruction order: input, window, logger |
| `Game` interface | ✅ | `OnStart/OnUpdate/OnFixedUpdate/OnRender/OnResize/OnShutdown` |
| Frame time, clamped dt, fixed-step accumulator | ✅ | |
| Fixed-step backlog check | ✅ | `accumulator >= fixed_step` after the loop |
| `OnResize` delivered after `OnStart` | ✅ | `glViewport` stays early |
| `OnShutdown` rule | ✅ | Called only if `OnStart` returned true, on every path; `m_app` reset on every exit |
| FPS in title | ✅ | |
| Minimized handling | ✅ | |
| Render interpolation alpha | ⏸ | |
| Exception thrown from `OnStart` | 🟡 | Caught by `Engine::Run`, but `OnShutdown` is not called and `m_app` is not reset. Acceptable under the rule above |

### Input

Policy: **feature-frozen**. Bug fixes only until the game demands more.

| Item | State | Notes |
|---|---|---|
| GLFW input backend, event queue | ✅ | |
| Physical key/mouse state with pressed/released | ✅ | |
| Raw input works before action finalization | ✅ | |
| Action map (digital, scalar, vector2), bindings | ✅ | |
| Action resolver | ✅ | Sub-frame taps now preserved (physical `pressed`/`released` are ORed in) |
| `InputFrame` snapshot, `Input` facade | ✅ | |
| Event queue overflow warning | ✅ | Oldest event is dropped on overflow |
| Mouse delta spike on first move / focus change | ✅ | Position validity resets on focus gain and loss |
| Mouse delta spike on cursor-mode change | ⏸ | Fix before mouse-look |
| Input contexts | 🟡 | Stack exists; `Resolve()` ignores `active_contexts` (C4100 accepted) |
| Consumption policy | ⬜ | Never read; real use is ImGui capture |

**Known input limitations (remaining)**

1. Cursor-mode change can still create a large mouse delta.
2. `ModifierKey::CapsLock/NumLock` can never be set (modifiers derive from key state).
3. `Disable` on a context removes it from the stack so `Enable` cannot restore its order.
4. Raw `WasKeyPressed` per rendered frame is unsafe inside a fixed step; use the intent pattern.
5. On queue overflow the oldest event is dropped, which could be a KeyUp (very unlikely at 4096 events).

### Logging

| Item | State | Notes |
|---|---|---|
| spdlog engine and game loggers, console + file sinks | ✅ | Files go to `out/build/x64-debug/game/logs/` |
| `LOG_TRACE`/`LOG_DEBUG` active in Debug | ✅ | |
| `logs/` directory exists at startup | ⬜ *(unverified)* | Run once from a clean checkout; spdlog's file sink normally creates it |
| One spdlog copy across exe and DLL | ⏸ | |

### Rendering, gameplay, tools

| Item | State |
|---|---|
| `Renderer2D` (ortho camera, batched colored quads) | ⬜ |
| `glViewport`/clear moved out of `Application` | ⬜ (temporary calls in `application.cpp`) |
| Textures / sprites (stb_image) | ⬜ |
| Player controller and platform collision (game code) | ⬜ |
| Input-intent struct in the game | ⬜ |
| ImGui tuning panel | ⬜ |
| Camera follow, draw order | ⬜ |
| Entity model decision, `Scene` | ⬜ |
| Animation, audio, assets | ⬜ |
| Tests for input logic | ⬜ |
| Packaging / clean-directory test | ⬜ |

---

## Open issues, prioritized

**Do now (tiny)**
1. Comment out the unused `dt` name in `PlatformerGame::OnUpdate`.

**M1a: Renderer2D**
4. `Renderer2D`: ortho camera (glm), batched colored quads, embedded shaders.
5. Move `glViewport` and clear out of `Application` into the renderer.

**Before the player (M1b)**
6. Write the input-intent struct pattern in the game (not the engine).
7. Add the ImGui tuning panel.

**Housekeeping**
8. README build steps; trim CMake presets.
9. Update this file after every milestone.

---

## Next steps (in order)

1. Apply open issues 1 to 3.
2. **M1a:** `Renderer2D` with ortho camera and batched quads; render a few rectangles.
3. **M1b:** player and platforms in game code with a fixed step and intent sampling.
4. ImGui tuning panel.
5. Input unit tests and a CI build.
6. Textures/sprites, then decide the entity model.

---

## Deferred register

Deferred on purpose. Each item has a trigger: do not start it before the trigger is true.

| Item | Why deferred | Trigger to revisit |
|---|---|---|
| Input contexts wired into `Resolve()` (optional context on bindings, invalid = always active) | No pause menu or UI yet | Pause menu or ImGui capture is needed |
| Input consumption policy | Never read | Same as above (ImGui `WantCaptureKeyboard/Mouse`) |
| Context `Disable`/`Enable` order restore | Depends on contexts being used | Same as above |
| Cursor-mode delta reset | Not needed for a platformer | Mouse-look, drag, or camera pan |
| `CapsLock`/`NumLock` modifiers | No use | A binding needs them |
| Controller / gamepad, remapping, text input handling | Not required for the first game | Playtesters need a gamepad, or a settings menu |
| Render interpolation alpha in `FrameTime` | Fixed 60 Hz looks fine at first | Movement visibly jitters on high-refresh monitors |
| Resizable windows and edge-drag freeze fix | `resizable = false` by default | The game needs a resizable window |
| Multiple windows (`glfwTerminate` in the window destructor) | One window only | A second window or tool window is needed |
| macOS support (`GLFW_OPENGL_FORWARD_COMPAT`) and Linux/macOS presets | Windows only for now | A non-Windows build is wanted |
| One spdlog copy across exe and DLL (exported logging function) | Only matters with a game DLL | Hot reload is attempted |
| Hot reload (`extern "C"` factory returning `Game*`) | Boundary is kept open, not implemented | Iteration time becomes painful; revisit at end of M3 |
| SHARED vs STATIC default | SHARED chosen for hot reload | End of M3 if hot reload is still unused |
| Exception from `OnStart` calling `OnShutdown` | Rare | `OnStart` starts doing fallible work (asset loading) |
| `GameInstance`, `World`, `GameMode` | Exist in Unreal for networking and level travel | Cross-scene state gets awkward to hold in `Game` |
| Entity model (plain structs + IDs, handles, or ECS such as EnTT) | Unproven; decide from real pain | After M1b, at the start of M2 |
| `Scene` and deferred spawn/destroy | Needs the entity model | M2 |
| AABB collision moved into the engine | Keep in game code until a second use | A second user of the same code appears |
| Physics backend | Own AABB first | AABB stops being enough (slopes, many bodies) |
| Textures / sprites / asset-path strategy | Colored quads first | M3 |
| Audio, animation, level transitions | Not needed for the prototype | When they improve the next playable milestone |
| RHI abstraction | One backend | A second graphics backend is planned |
| Editor, visual scripting, reflection/GC, plugin system | Non-goals | A concrete workflow demands one |
| Asset database, prefabs, scene serialization, save/load | No workflow needs them | After M3 |
| 2.5D features (depth, parallax, perspective, lighting) | 2D first | A representative game needs them (M6) |
| Packaging exe + DLLs + assets | Too early | M5 |
| CI build and tests | After the player exists | Once M1b lands |

---

## Change log

| Date | Change |
|---|---|
| 2026-10-20 | P0 hardening applied (visibility, spdlog level, event pump, window-owned callbacks, main cleanup). |
| 2026-10-20 | P1 applied: `Game`, `Application`, `Engine::Run`, frame time, fixed step, resize/minimize handling; `PlatformerGame` split into its own files. |
| 2026-10-20 | Architecture doc updated to 1.2; `STATUS.md` created. |
| 2026-10-20 | Review fixes applied: `OnResize` order, backlog check, includes, shutdown rule, `m_app` reset, glm as a submodule linked PUBLIC, game warnings, input sub-frame taps and mouse-delta spike. Deferred register added. |
