cmake_minimum_required(VERSION 3.16)
project(ECS VERSION 1.0.0 LANGUAGES CXX)

# ==== C++ Configuration ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==== CPM ====
include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

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
    src/systems/mario_systems.cpp
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
