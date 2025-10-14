# R-Type

This project aims to recreate the classic [R-Type](https://fr.wikipedia.org/wiki/R-Type) game with our own ECS game engine, and add a multiplayer mode to it.

We implemented a multi-threaded server using [Asio](https://think-async.com/Asio/) and a graphical client with [SFML](https://www.sfml-dev.org/).

##  Dependencies

[![SFML Logo](https://www.sfml-dev.org/download/goodies/sfml-icon.svg)](https://www.sfml-dev.org)


- [CMake](https://cmake.org/) **≥ 3.20**
> To check your CMake version: `cmake --version`
> If it's below 3.20, consider upgrading via your package manager or [CMake downloads](https://cmake.org/download/).
- **Windows**: [Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) (workload *Desktop development with C++*)
- **Linux**: `g++` or `clang++`, `make` (package `build-essential` on Debian/Ubuntu)

All third-party libraries (Asio, SFML) are automatically fetched with [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

> ⚠️ **Note**: You do **not** need to install SFML/Asio manually.

##  Compilation

This project uses **CMake presets** defined in [`CMakePresets.json`](./CMakePresets.json).
They provide ready-to-use configurations for **Windows (MSVC)** and **Linux (Makefiles)**.

>  **Note**: The project uses CPM cache (`.cache/CPM`) to avoid re-downloading dependencies on each build.

###  Windows (MSVC / Build Tools)

```powershell
# Clone the repository
git clone https://github.com/KBCanap/R-TYPE.git (Optionnal)<repo name>
cd <repo name>

# Configure for development (with tests)
cmake --preset win-config-dev

# Build Debug (dev with tests)
cmake --build --preset win-build-dev

# OR configure for release (production)
cmake --preset win-config-release

# Build Release
cmake --build --preset win-build-release
```

 **Binaries will be generated in:**
- **Dev**: `build/win/bin/Debug/` (with tests)
- **Release**: `build/win-release/bin/Release/` (production)

###  Linux (Makefiles + g++)

```bash
# Clone the repository
git clone <repo ulr>
cd <repo name>

# Configure for development (with tests)
cmake --preset linux-config-dev

# Build Dev (debug with tests)
cmake --build --preset linux-build-dev

# OR configure for release (production)
cmake --preset linux-config-release

# Build Release
cmake --build --preset linux-release

# Build Debug
cmake --build --preset linux-debug

```
> [!TIP]
> To speed up the compilation of the build on linux if you have a multicore cpu, you can add `-- -j$(nproc)`
> Like this: `cmake --build --preset linux-release -- -j$(nproc)`

> [!NOTE]
> You can still use the default `cmake` command
> For Linux: To configure the Makefile, run: `cmake -B build -S .`
> Then to compile `cd build && make -j$(nproc)` like for cmake **-j** is an option for multicore compilation

 **Binaries will be generated in:**
- **Dev**: `build/dev/bin/`
- **Release**: `build/linux/bin/`

## ▶️ Running

### Start the server:

**Windows:**
```powershell
# Dev build (with tests)
./build/win/bin/Debug/r-type_server.exe

# Release build (production)
./build/win-release/bin/Release/r-type_server.exe
```

**Linux:**
```bash
# Dev build (with tests)
./build/dev/bin/r-type_server

# Release build (production)
./build/linux/bin/r-type_server
```

### Start the client (in another terminal):

**Windows:**
```powershell
# Dev build (with tests)
./build/win/bin/Debug/r-type_client.exe

# Release build (production)
./build/win-release/bin/Release/r-type_client.exe
```

**Linux:**
```bash
# Dev build (with tests)
./build/dev/bin/r-type_client

# Release build (production)
./build/linux/bin/r-type_client
```

> [!WARNING]
> For the client to work you need to have the [assets](./assets/) folder within the same folder as the r-type_client binary

## 📂 Binary Locations Summary

| Platform | Configuration | Path |
|----------|---------------|------|
| Windows  | Dev (Debug + Tests) | `build/win/bin/Debug/` |
| Windows  | Release (Production) | `build/win-release/bin/Release/` |
| Linux    | Dev (Debug + Tests) | `build/dev/bin/` |
| Linux    | Release (Production) | `build/linux/bin/` |