cmake_minimum_required(VERSION 3.16)
project(ECS VERSION 1.0.0 LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fichiers sources ECS
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

# Création de la bibliothèque partagée
add_library(ecs SHARED ${ECS_SOURCES})

# Configuration des répertoires d'inclusion
target_include_directories(ecs PUBLIC include)

# Lier avec ASIO pour les types dans les headers network
if(TARGET asio_interface)
    target_link_libraries(ecs PUBLIC asio_interface)
endif()

# Installation
install(TARGETS ecs
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/ecs)