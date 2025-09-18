# R-Type

This project aims to recreate the classic [R-Type](https://fr.wikipedia.org/wiki/R-Type) game and add a multiplayer mode to it.

We implemented a multi-threaded server using [Asio](https://think-async.com/Asio/) and a graphical client with [SFML](https://www.sfml-dev.org/).

## 📦 Dependencies

- [CMake](https://cmake.org/) **≥ 3.20**
- **Windows**: [Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) (workload *Desktop development with C++*)
- **Linux**: `g++` or `clang++`, `make` (package `build-essential` on Debian/Ubuntu)

All third-party libraries (Asio, SFML) are automatically fetched with [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

> ⚠️ **Note**: You do **not** need to install SFML/Asio manually.

## ⚙️ Compilation

This project uses **CMake presets** defined in [`CMakePresets.json`](./CMakePresets.json).
They provide ready-to-use configurations for **Windows (MSVC)** and **Linux (Makefiles)**.

> 💡 **CI/CD Note**: The project uses CPM cache (`.cache/CPM`) to avoid re-downloading dependencies on each build. This directory should be cached in CI/CD pipelines.

### 🪟 Windows (MSVC / Build Tools)

```powershell
# Clone the repository
git clone https://github.com/your-username/R-Type.git
cd R-Type

# Configure with MSVC
cmake --preset win-msvc

# Build Debug
cmake --build --preset win-debug

# Build Release
cmake --build --preset win-release
```

✅ **Binaries will be generated in:**
- `build/win/bin/Debug/`
- `build/win/bin/Release/`

### 🐧 Linux (Makefiles + g++)

```bash
# Clone the repository
git clone https://github.com/your-username/R-Type.git
cd R-Type

# Configure with Makefiles
cmake --preset linux-make

# Build Release
cmake --build --preset linux-release
```

✅ **Binaries will be generated in:**
- `build/linux/bin/Release/`

## ▶️ Running

### Start the server:

**Windows:**
```powershell
./build/win/bin/Release/r-type_server.exe   # Release
./build/win/bin/Debug/r-type_server.exe     # Debug
```

**Linux:**
```bash
./build/linux/bin/Release/r-type_server
```

### Start the client (in another terminal):

**Windows:**
```powershell
./build/win/bin/Release/r-type_client.exe   # Release
./build/win/bin/Debug/r-type_client.exe     # Debug
```

**Linux:**
```bash
./build/linux/bin/Release/r-type_client
```

## 📂 Binary Locations Summary

| Platform | Configuration | Path |
|----------|---------------|------|
| Windows  | Debug         | `build/win/bin/Debug/` |
| Windows  | Release       | `build/win/bin/Release/` |
| Linux    | Release       | `build/linux/bin/Release/` |