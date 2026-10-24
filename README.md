# OpenGL-Game-Engine [2.5D]

A lite, code-first C++ game engine for 2D games, growing toward focused 2.5D features. You write gameplay in C++ against a small engine-owned lifecycle; there is no editor to install and no scripting layer to learn.

The engine is built game-first: a reference platformer (**Shadow Runner**, the `gamebox` executable) drives what gets implemented, and every other system is designed on paper first and built only when a real game needs it.

**Status:** Windows (MSVC) only, OpenGL 3.3 core. The reference game is playable: a scrolling level with coins, a spike, a goal and a patrolling enemy. See [`docs/STATUS.md`](docs/STATUS.md) for exactly what exists.

## Requirements

- Windows 10 or 11, x64
- Visual Studio 2022 with the **Desktop development with C++** workload (MSVC v143 and *C++ CMake tools for Windows*, which provide CMake and Ninja)
- Git
- A GPU and driver that support OpenGL 3.3 core

If you use a standalone CMake instead, it must be 3.24 or newer, with Ninja on your `PATH`.

## Clone (with submodules)

Third-party libraries are git submodules, so clone recursively:

```bat
git clone --recurse-submodules <repo-url>
cd OpenGL-Game-Engine
```

Already cloned without `--recurse-submodules`? Fetch them now:

```bat
git submodule update --init --recursive
```

| Submodule | Path | Used for |
|---|---|---|
| glfw | `external/glfw` | window and input |
| spdlog | `external/spdlog` | logging |
| glm | `external/glm` | math |
| imgui | `external/imgui/imgui` | debug panels |
| doctest | `external/doctest` | unit tests only |

The OpenGL loader (glad) is header-only and lives in the repository under `external/include`.

## Configure, build, run

The CMake presets call `cl.exe` and read `VSINSTALLDIR`, so run these commands from a **Visual Studio developer shell**: *x64 Native Tools Command Prompt for VS 2022* or *Developer PowerShell for VS 2022* (both are in the Start menu).

```bat
cmake --preset x64-debug
cmake --build --preset x64-debug
.\out\build\x64-debug\gamebox\gamebox.exe
```

For an optimized build use the `x64-release` preset in all three commands (the output folder becomes `out\build\x64-release`).

The build copies `engine.dll` next to `gamebox.exe`, so run the game from the build output or from the repository root. Logs are written to a `logs` folder under the directory you start it from.

**Using the Visual Studio IDE instead:** *File > Open > Folder* on the repository root, pick the **x64 Debug** configuration, then select `gamebox.exe` as the startup item and press F5.

| Preset | Purpose |
|---|---|
| `x64-debug` | Day-to-day development. Used by CI |
| `x64-release` | Optimized build. Used by CI |
| `x86-debug`, `x86-release` | Configure presets only; not built or tested in CI |
| `linux-debug`, `macos-debug` | Placeholders; those platforms are not supported yet |

## Run the tests

Unit tests cover pure logic (input, collision, player movement, level parsing). They need no window and no GPU.

```bat
ctest --preset x64-debug
```

Or run `out\build\x64-debug\tests\engine_tests.exe` directly to see doctest's output. CI runs the same presets on every push and pull request.

## Controls

| Key | Action |
|---|---|
| Left / Right | Move |
| Space | Jump (hold for a higher jump) |
| R | Restart the level |
| Esc | Quit |

Two ImGui panels are on by default: **Tuning** (live movement and enemy values, FPS, renderer stats) and **Stats** (game state and coins). Touch the spike or the enemy, or fall, and the screen tints red; reach the green goal and it tints green. Press R to go again.

## Project layout

```text
engine/        the engine (builds engine.dll)
  include/engine/   public headers: Application, Game, Input, Renderer, Camera, Scene, ...
  src/              private implementation (core, platform, input, renderer, runtime, gameplay)
gamebox/       the reference game, Shadow Runner (links only Engine::Engine)
tests/         doctest unit tests (no GL context needed)
external/      submodules and header-only third-party code
docs/          status, architecture and gameplay API design
```

## Documentation

- [`docs/STATUS.md`](docs/STATUS.md): what is implemented right now, known issues, deferred work
- [`docs/ENGINE_ARCHITECTURE_V1.md`](docs/ENGINE_ARCHITECTURE_V1.md): architecture, roadmap and decisions
- [`docs/GAMEPLAY_API.md`](docs/GAMEPLAY_API.md): the gameplay API (entities, scene, systems) and the lifecycle every game gets

## Troubleshooting

| Symptom | Cause and fix |
|---|---|
| CMake says a subdirectory such as `glfw` does not exist, or `external/` folders are empty | Submodules were not fetched. Run `git submodule update --init --recursive` |
| `Could not find include file ... SegmentHeap.cmake` or `cl.exe` not found | You are not in a Visual Studio developer shell. Open *x64 Native Tools Command Prompt for VS 2022* and run the commands there |
| `Ninja` not found | Install *C++ CMake tools for Windows* in the Visual Studio installer, or put Ninja on your `PATH` |
| `gamebox.exe` reports a missing `engine.dll` | Run the executable from `out\build\<preset>\gamebox\`, where the build copies the DLL |
| Window fails to open with an OpenGL error | The driver does not provide OpenGL 3.3 core. Update the GPU driver |
