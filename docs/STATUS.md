# Implementation Status

**Last updated:** 2026-10-20  
**Companion to:** [`ENGINE_ARCHITECTURE_V1.md`](ENGINE_ARCHITECTURE_V1.md) (intent). This file records what actually exists.

Legend: ✅ done · 🟡 partial / has known issues · ⬜ not started · ⏸ deferred on purpose

> Status is based on the last synced source and a successful build/run reported by the author. Items marked *(unverified)* were recommended but not confirmed as applied.

---

## Current milestone

**M0 (reliable build, loop, entry point): essentially done.**  
**Next: M1a (Renderer2D, first pixels).**

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
| Hidden visibility preset | ✅ | |
| `SPDLOG_ACTIVE_LEVEL` public definition | ✅ | TRACE in Debug, INFO otherwise |
| Warnings on `game` target | ⬜ | Unused `dt` in `OnUpdate` is currently invisible |
| `header-only-libraries` marked SYSTEM | ✅ | |
| glm dependency | ⬜ | `.gitignore` excludes `external/include/glm/*`; make it a submodule |
| `.gitignore` for `logs/`, `CMakeUserPresets.json` | ⬜ | |
| `pch.h` has `#pragma once` | ⬜ | `pch.cpp` is unused and can be deleted |
| README build instructions | ⬜ | `git clone --recurse-submodules`, preset, run |
| CMake presets cleanup (Linux/macOS are VS defaults) | ⬜ | |
| Tests / CI | ⬜ | |

### Platform and window

| Item | State | Notes |
|---|---|---|
| GLFW window, GL 3.3 core, glad loader | ✅ | |
| Window owns GLFW callbacks and user pointer | ✅ | Forwards to input backend |
| Input backend / window detach safety | ✅ | Either can be destroyed first |
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
| FPS in title | ✅ | |
| Minimized handling | ✅ | |
| Render interpolation alpha | ⏸ | Add only if movement jitters |
| `OnResize` delivered after `OnStart` | ⬜ *(unverified)* | Currently fires before `OnStart` |
| Standard includes in `application.cpp` | ⬜ *(unverified)* | `<chrono> <thread> <cstdio> <algorithm> <exception>` |
| Fixed-step backlog check | ⬜ *(unverified)* | Use `accumulator >= fixed_step` after the loop |
| Consistent `OnShutdown` rule | ⬜ *(unverified)* | Call only if `OnStart` returned true, on all failure paths |

### Input

Policy: **feature-frozen**. Bug fixes only until the game demands more.

| Item | State | Notes |
|---|---|---|
| GLFW input backend, event queue | ✅ | |
| Physical key/mouse state with pressed/released | ✅ | |
| Raw input works before action finalization | ✅ | |
| Action map (digital, scalar, vector2), bindings | ✅ | |
| Action resolver | 🟡 | See limitations below |
| `InputFrame` snapshot, `Input` facade | ✅ | |
| Event queue overflow warning | ✅ | Oldest event is dropped on overflow |
| Input contexts | 🟡 | Stack exists; `Resolve()` ignores `active_contexts` (C4100 warning accepted) |
| Consumption policy | ⬜ | Never read; real use is ImGui capture |
| Controller / gamepad, remapping, text input handling | ⏸ | |

**Known input limitations**

1. Action-level `pressed` can be lost when a key is tapped and released within one frame. Fix before jump buffering.
2. First mouse move, focus regain and cursor-mode change create a large delta (position starts at 0,0). Fix before mouse-look or drag.
3. `ModifierKey::CapsLock/NumLock` can never be set (modifiers derive from key state).
4. `Disable` on a context removes it from the stack so `Enable` cannot restore its order.
5. Raw `WasKeyPressed` per rendered frame is unsafe inside a fixed step; use the intent pattern.

### Logging

| Item | State | Notes |
|---|---|---|
| spdlog engine and game loggers, console + file sinks | ✅ | Files go to `logs/` |
| `LOG_TRACE`/`LOG_DEBUG` active in Debug | ✅ | |
| `logs/` directory exists at startup | ⬜ | Verify a clean checkout creates it or fails gracefully |
| One spdlog copy across exe and DLL | ⏸ | Matters only for a hot-reloaded game DLL |

### Rendering, gameplay, tools

| Item | State |
|---|---|
| `Renderer2D` (ortho camera, batched colored quads) | ⬜ |
| `glViewport`/clear moved out of `Application` | ⬜ (temporary calls in `application.cpp`) |
| Textures / sprites (stb_image) | ⬜ |
| Player controller and platform collision (game code) | ⬜ |
| ImGui tuning panel | ⬜ |
| Camera follow, draw order | ⬜ |
| Entity model decision, `Scene` | ⬜ |
| Animation, audio, assets | ⬜ |
| Tests for input logic | ⬜ |
| Packaging / clean-directory test | ⬜ |
| 2.5D features | ⏸ |
| `GameInstance` / `World` / `GameMode` | ⏸ (replaced by `Game` + future `Scene`) |

---

## Open issues, prioritized

**Do now (small, before more code builds on the loop)**
1. Move the first `OnResize` after `OnStart` and finalization (`glViewport` stays early).
2. Add missing standard includes to `application.cpp`.
3. Fix the fixed-step backlog check.
4. Make the `OnShutdown` rule consistent across failure paths.

**Do before the renderer**
5. Add glm as a submodule and fix `.gitignore` (`logs/`, `CMakeUserPresets.json`).
6. Add warnings to `game/CMakeLists.txt`.

**Do before the player**
7. Action-level `pressed` for sub-frame taps.
8. Write the input-intent struct pattern in the game (not the engine).

**Housekeeping**
9. `#pragma once` in `pch.h`; delete `pch.cpp`; fix the `End`/`CapsLock` line in `input_types.h`.
10. README build steps; trim CMake presets.
11. Update this file after every milestone.

---

## Next steps (in order)

1. Apply open issues 1 to 6 above.
2. **M1a:** `Renderer2D` with ortho camera and batched quads; render a few rectangles.
3. **M1b:** player and platforms in game code with a fixed step and intent sampling.
4. ImGui tuning panel.
5. Input unit tests and a CI build.
6. Textures/sprites, then decide the entity model.

---

## Change log

| Date | Change |
|---|---|
| 2026-10-20 | P0 hardening applied (visibility, spdlog level, event pump, window-owned callbacks, main cleanup). |
| 2026-10-20 | P1 applied: `Game`, `Application`, `Engine::Run`, frame time, fixed step, resize/minimize handling; `PlatformerGame` split into its own files. |
| 2026-10-20 | Architecture doc updated to 1.2; `STATUS.md` created. |
