# Copilot instructions

## Repository overview

This is a small C++20 OpenGL 2.5D game-engine prototype and its reference game, Shadow Runner. The root CMake project builds a shared `engine` DLL and a `game` executable. The engine owns the window, input, timing, fixed-step loop, logging, and OpenGL rendering; game code derives from `Engine::Game` and supplies lifecycle callbacks. The repository is roughly 100 tracked project/dependency files plus vendored Git submodules and third-party headers. It targets Windows/MSVC with Ninja; CMake minimum is 3.24. The source also contains vendored libraries with their own C/C++ standards and build files—do not modify them for normal engine/game work.

Dependencies are vendored/submodule-backed under `external/`: GLFW (windowing and input), spdlog (logging), GLM (math), and `external/include` (including glad and stb headers). A fresh checkout must use `git clone --recurse-submodules` or `git submodule update --init --recursive` before configuring.

## Layout and architecture

- `CMakeLists.txt`: root project; sets C++20, disables compiler extensions, and adds `external`, `engine`, and (when top-level) `game`.
- `CMakePresets.json`: configure presets. Windows presets use Ninja and `cl.exe`; the normal preset is `x64-debug` (also `x64-release`, `x86-*`). There are Linux/macOS configure presets, but they are not the primary supported environment.
- `engine/CMakeLists.txt`: shared `engine` target, exported as `Engine::Engine`, warning flags, PCH, and dependency links.
- `engine/include/engine`: public API. Keep GLFW/OpenGL/private implementation details out of these headers. Key entry points are `engine/game.h`, `engine/application.h`, `engine/input/input.h`, and `engine/renderer/renderer.h`.
- `engine/src`: private runtime, GLFW platform backend, input implementation, logger, and renderer/OpenGL helpers. `Application::Run` owns the order: events/input frame, `OnUpdate`, fixed updates, clear/render/swap, input end frame.
- `game/src/main.cpp`: executable entry point; creates `PlatformerGame` and calls `Engine::Run`. `PlatformerGame.cpp/.h` is the reference gameplay implementation.
- `external`: third-party submodules and the project’s header-only include target. Top-level CMake disables third-party docs, examples, tests, benchmarks, and installs.
- `docs/ENGINE_ARCHITECTURE_V1.md`: intended architecture and game-first rules. `docs/STATUS.md`: implementation status and known limitations; consult both before changing architecture.
- `out/`: generated build trees (ignored). `logs/`: runtime logs (ignored). Do not commit generated output, `.vs/`, `CMakeUserPresets.json`, or `notes/*`.

Maintain the dependency direction: game -> public engine API -> private engine implementation -> platform/third-party libraries. Keep the reference game buildable and playable after every change. Input is intentionally feature-frozen except for bug fixes; prefer putting game-specific behavior in `game` rather than prematurely generalizing the engine.

## Configure, build, test, and run

Run these commands from the repository root in a Visual Studio Developer PowerShell/Command Prompt where `cl.exe` is available. CMake 4.3.1-msvc1, Ninja 1.13.2, and MSVC 19.51 were verified here.

1. Initialize dependencies once: `git submodule update --init --recursive`.
2. Configure Debug: `cmake --preset x64-debug`. This creates `out/build/x64-debug` and uses Ninja. Always configure before the first build or after changing CMake files/presets.
3. Build: `cmake --build out/build/x64-debug --parallel 2`. There is no `buildPresets` entry, so `cmake --build --preset x64-debug` fails with “No such build preset”; use the binary-directory command above.
4. Release uses `cmake --preset x64-release`, followed by `cmake --build out/build/x64-release --parallel 2`.
5. Run manually from the build’s game directory so relative runtime files are located consistently: `Set-Location out/build/x64-debug/game; .\game.exe`. This opens a GLFW/OpenGL window; Escape exits. The program writes `logs/engine.log` and `logs/game.log` relative to its working directory. A headless cloud agent normally cannot meaningfully run this GUI program.
6. Tests: `ctest --test-dir out/build/x64-debug --output-on-failure`. The command is valid but currently reports `No tests were found!!!`; the top-level project has no registered test target. Do not interpret that as test coverage.

A verified Debug build completes successfully. Existing warnings are expected: MSVC C4100 for `active_contexts` in `engine/src/input/input_action_resolver.cpp` and unused `dt` in `game/src/PlatformerGame.cpp`; do not hide new warnings or assume these warnings are caused by a change. No top-level CI workflow, lint script, formatter configuration, or project test suite was found. `external/spdlog` contains its own formatting/lint files, but those apply only when working on that vendored dependency.

For a clean rebuild, remove only the generated tree (`Remove-Item -Recurse -Force out/build/x64-debug`), then rerun configure and build. Never remove source submodules or user changes. If CMake says compiler variables changed, it will regenerate the cache; rerun the build after configuration. Visual Studio may select a host/compiler architecture based on the active developer environment even when the preset is named x64, so use the appropriate x64 Developer environment when architecture matters.

## Change and validation guidance

Before editing, check `docs/STATUS.md` for known limitations and preserve public API boundaries. Prefer existing CMake targets and vendored dependencies; do not add package-manager downloads. For engine changes, build both affected compilation units through the normal full build. For gameplay changes, at minimum build and, when possible, run the GUI executable manually. Report the absence of automated tests explicitly. Check `git diff` and `git status` before finishing so generated `out/` or runtime logs are not included and unrelated existing user changes are not overwritten.

Trust these instructions for normal navigation and validation; search the repository only when they are incomplete or contradicted by the current files.

## Git Commit Message Generation

When generating a Git commit message, analyze the actual Git diff carefully and describe the implementation changes in detail.

### Required Format

- **First line:** A concise summary of the primary change, with a maximum of 50 characters.
- **Second line:** Completely blank.
- **Third line onward:** A detailed summary of the changes using bullet points.

### Detailed Change Analysis

- Explain what actually changed in the code, not just which files were modified.
- Describe significant changes to classes, functions, methods, data structures, algorithms, control flow, APIs, and system behavior.
- Explain the purpose and technical impact of each significant change when supported by the diff.
- For changes spanning multiple files, connect related modifications and explain how they work together.
- For C++ and OpenGL changes, identify relevant rendering operations, resource management, initialization sequences, error handling, lifecycle changes, and API behavior when applicable.
- For CMake changes, explain changes to targets, dependencies, compiler options, linking, and build configuration when applicable.
- Explicitly describe bug fixes, new features, refactoring, performance improvements, and removed functionality where applicable.
- Do not merely list modified filenames or provide generic statements such as "updated engine files" or "improved rendering."
- Do not invent functionality, motivations, or implementation details that cannot be inferred from the diff.
- Cover all significant changes without repeating the same information.

### Diff Inspection

1. Inspect the available staged and unstaged diffs before writing the commit message.
2. Identify the meaningful additions, deletions, and modifications in each relevant file.
3. Distinguish implementation changes from formatting-only changes, generated files, and unrelated modifications.
4. Summarize the overall purpose of the commit and then describe the important technical changes.
5. If the available diff does not contain enough information to explain a change, do not speculate.

### Output Requirements

- Output only the commit message, with no introductory text or explanations.
- Keep the first line at 50 characters or fewer.
- Leave exactly one blank line after the first line.
- Start the detailed summary on the third line.
- Use informative bullet points for the body.
- Scale the body length to the complexity of the changes: simple commits can have short bodies, while substantial multi-file changes must have detailed bodies.
- Prefer specific technical descriptions over generic summaries.
- Never claim to have inspected files or diffs that were not actually available to the generator.