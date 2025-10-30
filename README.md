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
- **SFML** \<=2.6.1

All third-party libraries (Asio, SFML) are automatically fetched with [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

> ⚠️ **Note**: You don’t need to install SFML or Asio manually, but system libraries are not handled automatically. CMake will let you know if any dependencies are missing and guide you through installing them.

> ⚠️ **Note**: On window, we recommand using vcpkg to install system libraries dependencies. You may take a look CMakePresets.json to se if the path in toolchainFile is correct for your vcpkg installation.

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

 > **Note**: The binaries are generated at the root of the repository

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
cmake --build --preset linux-build-release

```
> [!TIP]
> To speed up the compilation of the build on linux if you have a multicore cpu, you can add `-- -j$(nproc)`
> Like this: `cmake --build --preset linux-build-release -- -j$(nproc)`

> [!NOTE]
> You can still use the default `cmake` command
> For Linux: To configure the Makefile, run: `cmake -B build -S .`
> Then to compile `cd build && make -j$(nproc)` like for cmake **-j** is an option for multicore compilation

 > **Note**: The binaries are generated at the root of the repository

## ▶️ Running

### Start the server:

```bash
./r-type_server [-p <tcp_port>] [-u <udp_port>]
```

### Start the client (in another terminal):

```bash
./r-type_client [-i <server_ip>] [-p <tcp_port>] [-h]
```

> [!WARNING]
> For the client to work you need to have the [assets](./assets/) folder within the same folder as the r-type_app binary
