cmake_minimum_required(VERSION 3.16)
project(ECS VERSION 1.0.0 LANGUAGES CXX)

# ==== C++ Configuration ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==== CPM ====
include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

# === Sol2 ===
CPMFindPackage(
  NAME sol2
  VERSION 3.3.0
  GITHUB_REPOSITORY ThePhD/sol2
  GIT_TAG v3.3.0
  OPTIONS "SOL2_EXAMPLES OFF"
)

find_path(LUA_INCLUDE_DIR lua.h PATHS /usr/include /usr/local/include)
find_library(LUA_LIBRARY lua PATHS /usr/lib /usr/lib64 /usr/local/lib)

if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIBRARY)
    message(FATAL_ERROR "Lua not found. Make sure lua and lua-devel are installed.")
endif()

add_library(lua STATIC IMPORTED GLOBAL)
set_target_properties(lua PROPERTIES
  IMPORTED_LOCATION ${LUA_LIBRARY}
  INTERFACE_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR}
)

# ==== ECS Sources ====
set(ECS_SOURCES
    src/registery.cpp
    src/projectile_pattern.cpp
    src/ai_movement_pattern.cpp
    src/weapon.cpp
    # Systems sources
    src/systems/common.cpp
    src/systems/position_system.cpp
    src/systems/weapon_system.cpp
    src/systems/control_system.cpp
    src/systems/render_system.cpp
    src/systems/collision_system.cpp
    src/systems/audio_system.cpp
    src/systems/input_system.cpp
    src/systems/projectile_system.cpp
    src/systems/ai_input_system.cpp
    src/systems/score_system.cpp
    src/systems/health_system.cpp
    # Network sources
    src/NetworkSystem.cpp
    src/ANetworkManager.cpp
)

add_library(ecs SHARED ${ECS_SOURCES})

# ==== Includes ====
target_include_directories(ecs PUBLIC include)

# ==== Link Dependencies ====
target_link_libraries(ecs PUBLIC lua sol2)

# Link ASIO if available
if(TARGET asio_interface)
    target_link_libraries(ecs PUBLIC asio_interface)
endif()

# ==== Installation ====
install(TARGETS ecs
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include/ecs)
