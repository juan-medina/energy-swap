- The pxe engine under `external/pxe` is authored and maintained as part of this project. It is not a third-party dependency. It may be freely modified and evolved as the game progresses, as long as changes follow the same style and preserve behavior unless explicitly requested otherwise.

- Header/Implementation Split: Public API, templates, and inline methods are in the header; implementation logic is in the .cpp file.
- Include Order: Project headers first, then system/third-party headers, all sorted and grouped. Use angle brackets for dependencies, quotes for project headers.
- Explicit Includes: All used types (e.g., `std::string`, `std::vector`) are explicitly included, even if transitively included elsewhere.
- Section Comments: Use clear section comments (e.g., `// =============================================================================`) to group related members and methods.
- Access Control: Public, protected, and private members are clearly separated.
- Method Grouping: Related methods (event, audio, rendering, etc.) are grouped together for clarity.
- Const-Correctness: Use `const` and `[[nodiscard]]` where appropriate for getters and pure functions.
- Template Usage: Templates are used for generic event and scene registration APIs, defined inline in the header.
- Error Handling: All functions returning a result use `pxe::result` and propagate errors up, never exceptions.
- Logging: Use SPDLOG macros for debug/info logging, never for error propagation (errors are handled via result/error).
- Naming Conventions: Use snake_case for variables and methods, PascalCase for types/classes, and trailing underscores for private members (e.g., `name_`).

- Method Decomposition: Break down large or complex methods into smaller, focused helpers that each express a single idea.
- Static Methods: Any function that does not use instance state should be made static, except event handlers or callbacks that must remain instance methods.

- Comment Policy:
  - Do not add comments about what the code is doing or why something was changed (e.g., "now does X instead of Y").
  - Code should be self-explanatory through clear naming and structure; only add comments for non-obvious or complex logic.
  - Rationale for changes belongs in commit messages or code review, not in the code itself.
  - Preserve existing comments when refactoring, unless they are no longer accurate or relevant. Add new comments only for non-obvious logic; avoid comments that simply restate code.

- Copilot Behavior:
  - Make minimal, targeted edits; do not rewrite or reformat entire files unless explicitly requested.
  - Follow the existing structure, naming, and idioms of the file being edited.
  - When intent is unclear, prefer asking for clarification over guessing.
  - Do not introduce new dependencies, libraries, or abstractions unless explicitly requested.
  - Keep diffs small and avoid unrelated changes.
  - Generate code that compiles under C++20 and follows all project conventions.

- Refactoring Policy:
  - Do not change the functional behavior of code during refactoring unless explicitly requested or required.
  - Do not rename identifiers, reorder methods, or restructure files unless the request is specifically about organization.
  - Include Preservation: When refactoring, do not change the list of #include directives unless required for correctness or explicit includes.
  - Avoid “optimizing” or “simplifying” logic unless explicitly asked.

## Development Philosophy
- The codebase follows the YAGNI (You Aren't Gonna Need It) principle: only implement features when they are truly needed.
- Avoid speculative abstractions or overengineering; keep systems minimal until requirements demand more.
- Example: No ECS system is present—just a vector of components—because a full ECS is not currently required.

## Event System
- The engine provides an event system for decoupled communication between components and scenes.
- Events are plain structs with optional data fields.
- Use the app class (available to all components and scenes) to publish or subscribe to events.
- Event names use PascalCase.
- Event handlers that can fail should return `pxe::result<void>`.

## Modern C++ Practices
- Do not use old C++ or C patterns (e.g., no raw pointers, no custom vector/collection types, no manual memory management).
- Always prefer standard C++20 features and containers (e.g., `std::vector`, `std::unique_ptr`).
- No exceptions; no RTTI (`dynamic_cast`, `typeid`).
- Prefer `constexpr`, `std::span`, `std::optional`, and const-correctness.
- Use `[[nodiscard]]` where appropriate.

## Project Overview
- energy-swap is a C++20 game where players move colored energy units between battery nodes, aiming to empty or unify each battery.
- The project uses the pxe engine, which wraps raylib, raygui, and other libraries for cross-platform 2D game development.
- Main logic and assets are in `src/` and `resources/`.

## Architecture & Key Components
- `src/`: Game source code, organized by feature (e.g., `energy/`, `res/`, `web/`).
- `media/`: Raw source assets (sprites, music, fonts, etc.) used to generate distributable assets.
- `resources/`: Generated, optimized game assets shipped with the game. Do not edit directly; regenerate from `media/` as needed.
- `external/pxe/`: Game engine source and dependencies. This engine code is authored here and may be edited to add features or fixes that benefit this or future games.
- `CMakeLists.txt`: Main build configuration. Supports multiple platforms (Windows, Linux, macOS, WebAssembly).

## Build & Run
- Use CMake for all builds. Out-of-source builds are recommended (see `cmake-build-*` folders).
- Example build commands:
  - Windows (MSVC):
    cmake -B cmake-build-msvc-debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build cmake-build-msvc-debug
  - WebAssembly (Emscripten):
    emcmake cmake -B cmake-build-wsl2-emscripten-debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build cmake-build-wsl2-emscripten-debug
- Executables are output as `energy-swap` in the build directory.

## Testing & Debugging
- No formal test suite; manual playtesting is standard.
- Debug builds enable extra logging and assertions.
- Use the `resources/` directory for hot-reloading assets during development.

## Engine and Game Architecture
- Prefer C++20 features and modern idioms.
- Game logic is component-based; look for systems in `src/energy/`.
- Resource loading is centralized; see `src/res/` for asset management patterns.
- UI and rendering use raylib/raygui via pxe wrappers.
- All engine and third-party code is vendored under `external/`.

## Tooling
- The codebase uses clang-tidy with strict warning settings. All warnings must be cleared before committing code.
- The active clang-tidy rules are defined in the project configuration.
- Run clang-tidy as part of the workflow to ensure compliance with project standards.
- Use clang-format to format code and sort includes automatically.
- All includes must be necessary—remove unused includes and add explicit includes for all used types.
- All third-party and standard library includes must use angle brackets: `#include <...>`.
- Only game/project headers use quotes: `#include "..."` with relative paths.

## Error and Result Handling
- All error and result handling uses `pxe::result` and `pxe::error` (similar to C++23 `expected`, but project-specific).
- Do not use exceptions; avoid code that throws or relies on exception handling.
- When propagating errors, construct new errors from previous ones using the error constructor, but do not extract or log the previous error's message directly.
- Logging is done via spdlog. Use debug logs for development information, and only minimal info-level logs for user-facing events.
- Do not log errors directly; errors are propagated via result/error and logged at the top engine/stack with stack trace included.

## Rendering & Layout Philosophy
- The engine renders the entire game at a small, fixed resolution, then scales to fit the window/screen size.
- No aspect ratio distortion or black bars: the view is extended horizontally to show more content on wider screens.
- Input (e.g., mouse) is mapped/scaled to the game resolution.
- Game scenes receive a `layout` event on resolution changes, allowing them to reposition/move graphics and UI elements as needed.

## Integration & Dependencies
- You may edit `external/pxe` to evolve the engine for new features or improvements that benefit this or future games.
- To update third-party dependencies, use upstream pxe engine updates.
- For platform-specific issues, check CMake toolchain files and build scripts.

## Examples
- See `external/pxe/include/pxe/app.hpp` & `external/pxe/src/pxe/app.cpp` for an example of class & formatting conventions.
- See `src/energy/scenes/game.hpp` & `src/energy/scenes/game.cpp` for a scene structure.
- See `src/energy/components/battery_display.hpp` & `src/energy/components/battery_display.cpp` for a component structure.

## Additional Notes
- For controller/browser quirks, see the README.md troubleshooting section.
- All contributions must follow the style and structure of existing code.
- When in doubt, ask for clarification rather than making assumptions about intent or style.