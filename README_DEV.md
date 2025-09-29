# R-TYPE Development Guide - CMake Configurations

This document provides a comprehensive overview of all CMake presets available in this project, designed to support development across Windows and Linux platforms.

## Prerequisites

- **CMake**: Version 3.20 or higher
- **C++ Standard**: C++17
- **Windows**: Visual Studio 2022 (MSVC) with x64 architecture
- **Linux**: g++ compiler with Unix Makefiles support

## Configuration Presets Overview

### Development Configurations

#### `win-config-dev` - Windows Development
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Build Directory**: `build/win/`
- **Features**:
  - C++17 standard
  - MSVC 2022 compiler
  - CPM dependency caching in `.cache/CPM`
  - Runtime output in `build/win/bin/`
  - **Tests**: Enabled
  - Compile commands export for IDE integration

**Usage:**
```bash
cmake --preset=win-config-dev
```

#### `win-config-release` - Windows Release (Production)
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Build Directory**: `build/win-release/`
- **Features**:
  - C++17 standard
  - MSVC 2022 compiler
  - CPM dependency caching in `.cache/CPM`
  - Runtime output in `build/win-release/bin/`
  - **Tests**: Disabled
  - Optimized for production builds

**Usage:**
```bash
cmake --preset=win-config-release
```

#### `linux-config-release` - Linux Release Development
- **Platform**: Linux only
- **Generator**: Unix Makefiles
- **Build Directory**: `build/linux/`
- **Build Type**: Release
- **Features**:
  - Optimized release builds
  - g++ compiler
  - CPM dependency caching
  - Runtime output in `build/linux/bin/`
  - **Tests**: Disabled

**Usage:**
```bash
cmake --preset=linux-config-release
```

#### `linux-config-dev` - Enhanced Linux Development
- **Platform**: Linux only
- **Generator**: Unix Makefiles
- **Build Directory**: `build/dev/`
- **Build Type**: Debug
- **Features**:
  - Debug symbols and information
  - Compile commands export for IDE integration
  - Unit tests enabled (`RTYPE_BUILD_TESTS=ON`)
  - Warnings as errors disabled for easier development (`RTYPE_WERROR=OFF`)
  - Best preset for active development
  - **Tests**: Enabled

**Usage:**
```bash
cmake --preset=linux-config-dev
```

## Build Presets

### Windows Build Options

#### `win-build-dev` - Windows Development Build
```bash
cmake --build --preset=win-build-dev
```
- Builds debug configuration using `win-config-dev` preset
- Includes debug symbols, assertions, and tests

#### `win-build-release` - Windows Release Build
```bash
cmake --build --preset=win-build-release
```
- Builds optimized release configuration using `win-config-release` preset
- Full optimizations enabled, no tests
- 4 parallel jobs for faster builds

### Linux Build Options

#### `linux-build-release` - Linux Release Build
```bash
cmake --build --preset=linux-build-release
```
- Builds release configuration using `linux-config-release` preset
- Optimized for performance

#### `linux-build-dev` - Development Build
```bash
cmake --build --preset=linux-build-dev
```
- Uses `linux-config-dev` configuration
- Optimized for development workflow
- Includes unit tests

## Testing Framework

This project uses **Catch2 v3** as the testing framework. Tests are automatically discovered and registered with CTest.

### Running Tests

Tests are only available when using presets with `RTYPE_BUILD_TESTS=ON`:
- `win-config-dev`
- `linux-config-dev`

**Configure with tests:**
```bash
# Windows
cmake --preset=win-config-dev

# Linux (development)
cmake --preset=linux-config-dev
```

**Build tests:**
```bash
# Windows
cmake --build --preset=win-build-dev

# Linux (development)
cmake --build --preset=linux-build-dev
```

**Run tests:**
```bash
# Windows (from build directory)
cd build/win
ctest

# Linux (development, from build directory)
cd build/dev
ctest

# Or run from project root
ctest --build-config Debug --test-dir build/dev  # Linux
ctest --build-config Debug --test-dir build/win  # Windows
```

### Automated Testing for Development

For automated testing during development, always use the debug/dev configurations:

**Linux:**
```bash
# Configure for development with tests
cmake --preset=linux-config-dev

# Build with debug symbols and tests
cmake --build --preset=linux-build-dev

# Run tests
cd build/dev && ctest
```

**Windows:**
```bash
# Configure for development with tests
cmake --preset=win-config-dev

# Build with debug symbols and tests
cmake --build --preset=win-build-dev

# Run tests
cd build/win && ctest
```

### Adding New Tests

1. **Create test file** in `tests/` directory:
```cpp
// tests/my_feature.test.cpp
#include <catch2/catch_test_macros.hpp>

// Your code to test
int multiply(int a, int b) { return a * b; }

TEST_CASE("Multiplication works", "[math]") {
    REQUIRE(multiply(2, 3) == 6);
    REQUIRE(multiply(-1, 5) == -5);
    REQUIRE(multiply(0, 10) == 0);
}

TEST_CASE("Edge cases", "[math][edge]") {
    REQUIRE(multiply(1, 1) == 1);
    REQUIRE(multiply(-1, -1) == 1);
}
```

2. **Add to CMakeLists.txt** in `tests/CMakeLists.txt`:
```cmake
set(TEST_SOURCES
  example.test.cpp
  my_feature.test.cpp  # Add your test file here
  # Add other test files here...
)
```

3. **Rebuild and run:**
```bash
# Rebuild tests
cmake --build --preset=linux-build-dev  # or your preferred preset

# Run tests
cd build/dev && ctest  # or your preferred test directory

# Run specific tests by regex
ctest -R "math"
```

### Test Organization Tips

- **Use descriptive test names**: `TEST_CASE("Should handle empty input gracefully", "[validation]")`
- **Group related tests with tags**: `[math]`, `[network]`, `[graphics]`
- **One feature per file**: `player.test.cpp`, `networking.test.cpp`
- **Test both success and failure cases**

Example test structure:
```cpp
TEST_CASE("Player movement", "[player][movement]") {
    SECTION("moves right when right key pressed") {
        // test implementation
    }
    
    SECTION("cannot move outside game bounds") {
        // test implementation
    }
}
```

## Recommended Workflows

### Development Workflow (Linux)
```bash
# Setup development environment with tests
cmake --preset=linux-config-dev

# Build in debug mode with tests
cmake --build --preset=linux-build-dev

# Run tests to verify everything works
cd build/dev && ctest

# Run specific test categories
ctest -R "networking"
```

### Development Workflow (Windows)
```bash
# Setup development environment with tests
cmake --preset=win-config-dev

# Build debug version with tests
cmake --build --preset=win-build-dev

# Run tests
cd build/win && ctest
```

### Production Build Workflow
```bash
# Linux production build (no tests)
cmake --preset=linux-config-release
cmake --build --preset=linux-build-release

# Windows production build (no tests)
cmake --preset=win-config-release
cmake --build --preset=win-build-release
```

## Key Variables Explained

- **`CMAKE_CXX_STANDARD`**: Set to 17 for C++17 support across all configurations
- **`CMAKE_RUNTIME_OUTPUT_DIRECTORY`**: Centralizes executable output for easier access
- **`CPM_SOURCE_CACHE`**: Caches downloaded dependencies to speed up builds
- **`RTYPE_WERROR`**: Controls whether warnings are treated as errors
- **`RTYPE_BUILD_TESTS`**: Enables/disables test compilation and Catch2 integration
- **`CMAKE_EXPORT_COMPILE_COMMANDS`**: Generates compile_commands.json for IDE integration

## Troubleshooting

### Common Issues

1. **CMake version too old**: Ensure CMake 3.20+ is installed
2. **Preset not found**: Verify you're running from the project root directory
3. **Build failures**: Check that `RTYPE_WERROR=ON` isn't causing issues with warnings
4. **Cache issues**: Clear CPM cache or build directories if dependencies seem outdated
5. **Tests not found**: Make sure you're using a test-enabled preset (`*-dev`)

### Platform-Specific Notes

- **Windows**: Requires Visual Studio 2022 with C++ workload installed
- **Linux**: Requires g++ and make installed (`build-essential` package on Ubuntu/Debian)

## Directory Structure

```
project-root/
├── build/
│   ├── win/           # Windows dev builds (debug + tests)
│   ├── win-release/   # Windows release builds (production)
│   ├── linux/         # Linux release builds  
│   └── dev/           # Development builds (Linux, debug + tests)
├── tests/
│   ├── CMakeLists.txt # Test configuration
│   ├── example.test.cpp
│   └── *.test.cpp     # Your test files
├── .cache/
│   └── CPM/           # CPM dependency cache
└── CMakePresets.json  # This configuration file
```

### Configure Presets
| Preset | Platform | Purpose | Tests |
|--------|----------|---------|-------|
| `win-config-dev` | Windows | Development with debug + tests | ✅ |
| `win-config-release` | Windows | Production release | ❌ |
| `linux-config-dev` | Linux | Development with debug + tests | ✅ |
| `linux-config-release` | Linux | Production release | ❌ |

### Build Presets
| Preset | Configuration | Platform |
|--------|---------------|----------|
| `win-build-dev` | Debug + Tests | Windows |
| `win-build-release` | Release | Windows |
| `linux-build-dev` | Debug + Tests | Linux |
| `linux-build-release` | Release | Linux |