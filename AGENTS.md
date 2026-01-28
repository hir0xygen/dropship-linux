# AGENTS.md - Dropship

Guidelines for AI coding agents working on this repository.

## Project Overview

**Dropship** is a portable Windows application for Overwatch 2 server selection.
It uses Windows Firewall rules to block game servers, allowing users to prefer specific regions.

- **Language**: C++20 (ISO C++latest)
- **Platform**: Windows x64 (requires administrator privileges)
- **UI Framework**: Dear ImGui (docking branch) with DirectX 11 backend
- **Build System**: MSBuild / Visual Studio 2022 (v143 toolset)

## Build Commands

### Prerequisites
- Visual Studio 2022 with C++ Desktop workload
- Windows SDK 10.0

### Build (Command Line)
```bash
# From repository root, navigate to solution directory
cd dropship

# Build Release x64 (recommended)
msbuild dropship.sln /m /p:Configuration=Release /p:Platform=x64

# Build Debug x64
msbuild dropship.sln /m /p:Configuration=Debug /p:Platform=x64
```

### Output Location
- Release: `dropship/x64/Release/dropship.exe`
- Debug: `dropship/x64/Debug/dropship.exe`

### Visual Studio
Open `dropship/dropship.sln` in Visual Studio 2022.

## Testing

This project does not have automated tests. Verify changes by:
1. Building successfully with no errors/warnings
2. Running the executable (requires admin rights)
3. Testing affected functionality manually

## Code Style Guidelines

### File Structure
```
dropship/dropship/
├── src/
│   ├── App.cpp/h          # Main application rendering
│   ├── main.cpp           # Entry point, global initialization
│   ├── pch.h              # Precompiled header (always include first)
│   ├── theme.h            # UI theming
│   ├── core/              # Core business logic classes
│   └── util/              # Utility functions and helpers
└── vendor/                # Third-party libraries (do not modify)
    ├── imgui-docking/     # Dear ImGui
    ├── json/              # nlohmann/json
    ├── asio/              # Networking
    └── stb_image/         # Image loading
```

### Indentation & Formatting
- **Indentation**: Tabs (not spaces)
- **Brace style**: Opening brace on same line as statement
- **Line endings**: CRLF (Windows)

```cpp
// Correct
if (condition) {
	doSomething();
} else {
	doOther();
}

// Incorrect
if (condition)
{
    doSomething();
}
```

### Includes
1. Always include `pch.h` first in `.cpp` files
2. Use quotes for project headers, angle brackets for system/vendor
3. Group: pch.h, then project headers, then standard library (via pch.h)

```cpp
#include "pch.h"

#include "core/Dashboard.h"
#include "util/trim.h"
```

### Naming Conventions
- **Classes**: `PascalCase` (e.g., `Dashboard`, `Firewall`, `PopupButton`)
- **Functions/Methods**: `camelCase` (e.g., `render()`, `tryWriteSettingsToFirewall()`)
- **Member variables**: `_camelCase` with underscore prefix (e.g., `_coInitilizeSuccess`)
- **Private static consts**: `__SCREAMING_SNAKE` double underscore prefix (e.g., `__UPDATE_VERSION_URI`)
- **Global variables**: `g_camelCase` (e.g., `g_settings`, `g_firewall`, `g_dashboard`)
- **Namespaces**: `snake_case` (e.g., `util::win_firewall`)

### Types & Modern C++
- Use C++20 features: `std::format`, `std::println`, `<print>`
- Prefer `std::unique_ptr` for ownership
- Use `std::optional` for nullable values
- Use `std::future` for async operations
- Use designated initializers for structs: `{ .title = "...", .action = []() {} }`

```cpp
std::unique_ptr<Settings> g_settings;
std::optional<std::string> tryFetchSettings();
std::future<void> update_future;
```

### Header Guards
Use `#pragma once` (not include guards).

### Error Handling
- Throw `std::runtime_error` for fatal initialization errors
- Use `SUCCEEDED()`/`FAILED()` macros for COM HRESULT checks
- Use `printf()` or `std::println()` for debug logging
- Wrap debug-only code in `#ifdef _DEBUG`

```cpp
if (FAILED(hr)) {
	printf("Operation failed\n");
	return;
}

#ifdef _DEBUG
	util::timer::Timer timer("operation_name");
#endif
```

### COM and Windows APIs
- Initialize COM in constructor, uninitialize in destructor
- Use `CComPtr<T>` and `CComBSTR` for COM smart pointers
- Check `SUCCEEDED()` before proceeding with COM operations

### ImGui Patterns
- Use static const for window flags
- Check global pointers before dereferencing: `if (g_xxx) (*g_xxx).render()`
- Use `ImGui::` namespace explicitly

### Comments
- Use `// comment` for single line
- Use `/* ... */` for multi-line or inline
- Mark TODOs: `// TODO description`
- Add safety warnings for critical operations: `/* !important ... */`

## Global Variables

Defined in `main.cpp`, accessed via `extern`:
```cpp
extern std::unique_ptr<Firewall> g_firewall;
extern std::unique_ptr<Dashboard> g_dashboard;
extern std::unique_ptr<Settings> g_settings;
extern std::unique_ptr<Updater> g_updater;
extern std::unique_ptr<Tunneling> g_tunneling;
extern std::unique_ptr<WindowWatcher> g_window_watcher;
extern ImFont* font_title;
extern ImFont* font_subtitle;
extern ImFont* font_text;
```

## Vendor Libraries (Do Not Modify)

- **imgui-docking**: UI framework - use `IMGUI_DEFINE_MATH_OPERATORS`
- **nlohmann/json**: JSON parsing - use `using json = nlohmann::json;`
- **asio**: Async networking (header-only)
- **stb_image**: Image loading

## Security Notes

- Application requires administrator privileges (UAC manifest)
- Firewall rules must have `RemoteAddresses` set before enabling to avoid blocking all traffic
- Never commit sensitive data or credentials

## CI/CD

GitHub Actions workflow (`.github/workflows/msbuild.yml`):
- Triggers on push to main
- Builds Release x64 on `windows-latest`
- Creates GitHub releases with `dropship.exe`

## Landing the Plane (Session Completion)

**When ending a work session**, you MUST complete ALL steps below. Work is NOT complete until `git push` succeeds.

**MANDATORY WORKFLOW:**

1. **File issues for remaining work** - Create issues for anything that needs follow-up
2. **Run quality gates** (if code changed) - Tests, linters, builds
3. **Update issue status** - Close finished work, update in-progress items
4. **PUSH TO REMOTE** - This is MANDATORY:
   ```bash
   git pull --rebase
   bd sync
   git push
   git status  # MUST show "up to date with origin"
   ```
5. **Clean up** - Clear stashes, prune remote branches
6. **Verify** - All changes committed AND pushed
7. **Hand off** - Provide context for next session

**CRITICAL RULES:**
- Work is NOT complete until `git push` succeeds
- NEVER stop before pushing - that leaves work stranded locally
- NEVER say "ready to push when you are" - YOU must push
- If push fails, resolve and retry until it succeeds
