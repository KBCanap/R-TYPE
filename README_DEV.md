# R-TYPE Development Guide - CMake Configurations

This document provides a comprehensive overview of all CMake presets available in this project, designed to support development across Windows and Linux platforms, as well as CI/CD workflows.

## Prerequisites

- **CMake**: Version 3.20 or higher
- **C++ Standard**: C++17
- **Windows**: Visual Studio 2022 (MSVC) with x64 architecture
- **Linux**: g++ compiler with Unix Makefiles support

## Configuration Presets Overview

### Development Configurations

#### `win-msvc` - Windows Development
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Build Directory**: `build/win/`
- **Features**:
  - C++17 standard
  - MSVC 2022 compiler
  - CPM dependency caching in `.cache/CPM`
  - Runtime output in `build/win/bin/`
  - **Tests**: Disabled

**Usage:**
```bash
cmake --preset=win-msvc
```

#### `win-msvc-tests` - Windows Development with Tests
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Build Directory**: `build/win/`
- **Features**:
  - Inherits from `win-msvc`
  - Unit tests enabled (`RTYPE_BUILD_TESTS=ON`)
  - Compile commands export for IDE integration
  - **Tests**: Enabled

**Usage:**
```bash
cmake --preset=win-msvc-tests
```

#### `linux-make` - Linux Release Development
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
cmake --preset=linux-make
```

#### `dev-linux` - Enhanced Linux Development
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
cmake --preset=dev-linux
```

### CI/CD Configurations

#### `ci-linux` - Linux CI/CD
- **Platform**: Linux only
- **Generator**: Unix Makefiles
- **Build Directory**: `build/ci/`
- **Build Type**: Release
- **Features**:
  - Optimized for CI environments
  - CPM cache in `/tmp/cpm_cache` for better CI performance
  - Warnings treated as errors (`RTYPE_WERROR=ON`)
  - Unit tests enabled (`RTYPE_BUILD_TESTS=ON`)
  - Compile commands exported
  - CI-specific optimizations (`RTYPE_CI_BUILD=ON`)
  - **Tests**: Enabled

**Usage:**
```bash
cmake --preset=ci-linux
```

#### `ci-windows` - Windows CI/CD
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Build Directory**: `build/ci-win/`
- **Features**:
  - Optimized for Windows CI environments
  - CPM cache in `C:/cpm_cache`
  - Warnings treated as errors (`RTYPE_WERROR=ON`)
  - Unit tests enabled (`RTYPE_BUILD_TESTS=ON`)
  - CI-specific optimizations (`RTYPE_CI_BUILD=ON`)
  - **Tests**: Enabled

**Usage:**
```bash
cmake --preset=ci-windows
```

## Build Presets

### Windows Build Options

#### `win-debug` - Windows Debug Build
```bash
cmake --build --preset=win-debug
```
- Builds debug configuration using `win-msvc` preset
- Includes debug symbols and assertions

#### `win-release` - Windows Release Build
```bash
cmake --build --preset=win-release
```
- Builds optimized release configuration using `win-msvc` preset
- Full optimizations enabled

#### `win-tests-build` - Windows Tests Build
```bash
cmake --build --preset=win-tests-build
```
- Builds debug configuration using `win-msvc-tests` preset
- Includes unit tests compilation

### Linux Build Options

#### `linux-release` - Linux Release Build
```bash
cmake --build --preset=linux-release
```
- Builds release configuration using `linux-make` preset
- Optimized for performance

#### `dev-build` - Development Build
```bash
cmake --build --preset=dev-build
```
- Uses `dev-linux` configuration
- Optimized for development workflow
- Includes unit tests

### CI/CD Build Options

#### `ci-linux-build` - CI Linux Build
```bash
cmake --build --preset=ci-linux-build
```
- Release build with 4 parallel jobs
- Optimized for CI performance
- Includes unit tests

#### `ci-windows-release` - CI Windows Build
```bash
cmake --build --preset=ci-windows-release
```
- Release build for Windows CI
- 4 parallel jobs for faster build times
- Includes unit tests

## Test Presets

### `win-tests` - Windows Unit Tests
```bash
ctest --preset=win-tests
```
- Runs unit tests on Windows
- Uses debug configuration
- Verbose output with failure details
- Requires `win-msvc-tests` configure preset

### `dev-tests` - Development Tests (Linux)
```bash
ctest --preset=dev-tests
```
- Runs tests with verbose output
- Uses debug configuration from `dev-linux`
- Ideal for development and debugging

### `ci-tests` - CI Unit Tests
```bash
ctest --preset=ci-tests
```
- Runs unit tests in CI environment
- Uses release configuration from `ci-linux`
- Shows output only on failure

## Package Presets

### `ci-package-linux` - Linux Packaging
```bash
cpack --preset=ci-package-linux
```
- Generates TGZ and DEB packages
- Uses CI Linux configuration

### `ci-package-windows` - Windows Packaging
```bash
cpack --preset=ci-package-windows
```
- Generates ZIP packages
- Uses CI Windows configuration

## Testing Framework

This project uses **Catch2 v3** as the testing framework. Tests are automatically discovered and registered with CTest.

### Running Tests

Tests are only available when using presets with `RTYPE_BUILD_TESTS=ON`:
- `win-msvc-tests`
- `dev-linux`
- `ci-linux`
- `ci-windows`

**Configure with tests:**
```bash
# Windows
cmake --preset=win-msvc-tests

# Linux (development)
cmake --preset=dev-linux

# Linux (CI)
cmake --preset=ci-linux
```

**Build tests:**
```bash
# Windows
cmake --build --preset=win-tests-build

# Linux (development)
cmake --build --preset=dev-build

# Linux (CI)
cmake --build --preset=ci-linux-build
```

**Run tests:**
```bash
# Windows
ctest --preset=win-tests

# Linux (development)
ctest --preset=dev-tests

# Linux (CI)
ctest --preset=ci-tests
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
cmake --build --preset=dev-build  # or your preferred preset

# Run tests
ctest --preset=dev-tests  # or your preferred test preset

# Run specific tests by tag
ctest --preset=dev-tests -R "math"
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
cmake --preset=dev-linux

# Build in debug mode with tests
cmake --build --preset=dev-build

# Run tests to verify everything works
ctest --preset=dev-tests

# Run specific test categories
ctest --preset=dev-tests -R "networking"
```

### Development Workflow (Windows)
```bash
# Setup development environment with tests
cmake --preset=win-msvc-tests

# Build debug version with tests
cmake --build --preset=win-tests-build

# Run tests
ctest --preset=win-tests
```

### Production Build Workflow
```bash
# Linux production build (no tests)
cmake --preset=linux-make
cmake --build --preset=linux-release

# Windows production build (no tests)
cmake --preset=win-msvc
cmake --build --preset=win-release
```

### CI/CD Workflow (Linux)
```bash
# Configure for CI with tests
cmake --preset=ci-linux

# Build optimized release with tests
cmake --build --preset=ci-linux-build

# Run tests
ctest --preset=ci-tests

# Package for distribution
cpack --preset=ci-package-linux
```

### CI/CD Workflow (Windows)
```bash
# Configure for CI with tests
cmake --preset=ci-windows

# Build release with tests
cmake --build --preset=ci-windows-release

# Package for distribution
cpack --preset=ci-package-windows
```

## Key Variables Explained

- **`CMAKE_CXX_STANDARD`**: Set to 17 for C++17 support across all configurations
- **`CMAKE_RUNTIME_OUTPUT_DIRECTORY`**: Centralizes executable output for easier access
- **`CPM_SOURCE_CACHE`**: Caches downloaded dependencies to speed up builds
- **`RTYPE_CI_BUILD`**: Enables CI-specific optimizations and behaviors
- **`RTYPE_WERROR`**: Controls whether warnings are treated as errors
- **`RTYPE_BUILD_TESTS`**: Enables/disables test compilation and Catch2 integration
- **`CMAKE_EXPORT_COMPILE_COMMANDS`**: Generates compile_commands.json for IDE integration

## Troubleshooting

### Common Issues

1. **CMake version too old**: Ensure CMake 3.20+ is installed
2. **Preset not found**: Verify you're running from the project root directory
3. **Build failures on CI**: Check that `RTYPE_WERROR=ON` isn't causing issues with warnings
4. **Cache issues**: Clear CPM cache or build directories if dependencies seem outdated
5. **Tests not found**: Make sure you're using a test-enabled preset (`*-tests` or `dev-*` or `ci-*`)

### Platform-Specific Notes

- **Windows**: Requires Visual Studio 2022 with C++ workload installed
- **Linux**: Requires g++ and make installed (`build-essential` package on Ubuntu/Debian)
- **CI environments**: Use dedicated CI presets for optimal performance and caching

## Directory Structure

```
project-root/
├── build/
│   ├── win/           # Windows MSVC builds
│   ├── linux/         # Linux release builds  
│   ├── dev/           # Development builds (Linux)
│   ├── ci/            # CI Linux builds
│   └── ci-win/        # CI Windows builds
├── tests/
│   ├── CMakeLists.txt # Test configuration
│   ├── example.test.cpp
│   └── *.test.cpp     # Your test files
├── .cache/
│   └── CPM/           # CPM dependency cache
└── CMakePresets.json  # This configuration file
```

# RECAP
### Configure Presets
| Preset | Platform | Purpose | Tests |
|--------|----------|---------|-------|
| `win-msvc` | Windows | Standard development | ❌ |
| `win-msvc-tests` | Windows | Development with tests | ✅ |
| `linux-make` | Linux | Production release | ❌ |
| `dev-linux` | Linux | Development with tests | ✅ |
| `ci-linux` | Linux | CI/CD pipeline | ✅ |
| `ci-windows` | Windows | CI/CD pipeline | ✅ |

### Build Presets
| Preset | Configuration | Platform |
|--------|---------------|----------|
| `win-debug` | Debug | Windows |
| `win-release` | Release | Windows |
| `win-tests-build` | Debug + Tests | Windows |
| `linux-release` | Release | Linux |
| `dev-build` | Debug + Tests | Linux |
| `ci-linux-build` | Release + Tests | Linux CI |
| `ci-windows-release` | Release + Tests | Windows CI |