cmake_minimum_required(VERSION 3.20)
project(RTYPE LANGUAGES CXX)

# ==== Language & tools ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/CPM.cmake)

# Asio (standalone, header-only)
CPMAddPackage(
  NAME asio
  VERSION 1.28.0
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-28-0
  OPTIONS
    "ASIO_NO_DEPRECATED ON"
)

# Find SFML
find_package(sfml-graphics QUIET)
find_package(sfml-window QUIET)
find_package(sfml-system QUIET)
find_package(sfml-audio QUIET)
find_package(sfml-network QUIET)

# If SFML is not found via find_package, try pkg-config
if(NOT sfml-graphics_FOUND OR NOT sfml-window_FOUND OR NOT sfml-system_FOUND OR NOT sfml-audio_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SFML REQUIRED sfml-graphics>=2.5 sfml-window>=2.5 sfml-system>=2.5 sfml-audio>=2.5 sfml-network>=2.5)
endif()

# Interface to properly expose Asio include (SYSTEM to avoid warnings)
add_library(asio_interface INTERFACE)
if(asio_ADDED)
  target_include_directories(asio_interface SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
else()
  # Fallback if Asio is already installed on the system
  find_package(asio REQUIRED)
  target_link_libraries(asio_interface INTERFACE asio::asio)
endif()

# Common definitions
add_compile_definitions(ASIO_STANDALONE ASIO_NO_DEPRECATED)

# ==== Utility function for target configuration ====
function(setup_target_warnings target)
  target_compile_options(${target} PRIVATE
    -Wall -Wextra -Wpedantic
    # Suppression of strict warnings that cause problems with ASIO
    -Wold-style-cast      # C-style cast
    -Woverloaded-virtual  # Virtual function masking
    -Wnull-dereference    # Null pointer dereference
  )
  # Link with threads on Unix
  find_package(Threads REQUIRED)
  target_link_libraries(${target} PRIVATE Threads::Threads)
endfunction()

# ==== Subprojects ====
# ECS Library
add_subdirectory(ecs)

# App (R-Type client with ECS)
add_subdirectory(app)

# ==== Main targets ====
# Server
set(SERVER_SOURCES
    Server/src/main.cpp
    Server/src/StartServer.cpp
    Server/src/GameSession.cpp
    Server/src/ClientState.cpp
    Server/src/Protocole.cpp
    Server/src/Tcpserver.cpp
    Server/src/UdpServer.cpp
    Server/src/MessageQueue.cpp
    Server/src/GameServerLoop.cpp
    Server/src/UdpProtocole.cpp
    Server/src/GameLogic.cpp
    ecs/src/registery.cpp
)

add_executable(r-type_server ${SERVER_SOURCES})
target_include_directories(r-type_server PRIVATE
  Server/include
)
target_link_libraries(r-type_server PRIVATE
  asio_interface
)

target_include_directories(r-type_server PRIVATE
    include
    ./ecs/include
)

# Lier avec les bibliothèques nécessaires
target_link_libraries(r-type_server PRIVATE
    ecs
)


# Warning and debug configuration
setup_target_warnings(r-type_server)
target_compile_definitions(r-type_server PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:Debug>:RTYPE_DEBUG>
)

# Client is now handled by the app module

# ==== Unit tests (optional) ====
if(RTYPE_BUILD_TESTS)
  enable_testing()

  # Catch2 via CPM (v3)
  CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    VERSION 3.5.3
    OPTIONS
      "CATCH_INSTALL_DOCS OFF"
      "CATCH_INSTALL_EXTRAS ON" # for include(Catch) and catch_discover_tests
  )
  if(Catch2_ADDED)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  endif()

  # Add your tests here
  add_subdirectory(tests)

  message(STATUS "Unit tests enabled")
endif()

# ==== Optimizations by configuration (fixed for multi-config) ====
# Avoid CMAKE_BUILD_TYPE here and use generator expressions:
# Release: -O3 -DNDEBUG only
target_compile_options(r-type_server PRIVATE
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ==== Installation (for packaging/deployment) ====
include(GNUInstallDirs)

install(TARGETS r-type_app r-type_server
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT Runtime
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "R-Type")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "R-Type multiplayer game")

# Linux-specific configuration
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgcc-s1, libstdc++6")

include(CPack)

# ==== Build information ====
message(STATUS "=== R-Type Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CPM cache: ${CPM_SOURCE_CACHE}")
message(STATUS "Warnings as errors: ${RTYPE_WERROR}")
message(STATUS "CI build: ${RTYPE_CI_BUILD}")
message(STATUS "Build tests: ${RTYPE_BUILD_TESTS}")
message(STATUS "===================================")