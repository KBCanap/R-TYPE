cmake_minimum_required(VERSION 3.16)
project(ECS VERSION 1.0.0 LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fichiers sources ECS
set(ECS_SOURCES
    src/registery.cpp
    src/systems.cpp
    src/projectile_pattern.cpp
    src/ai_movement_pattern.cpp
    src/weapon.cpp
)

# Création de la bibliothèque partagée
add_library(ecs SHARED ${ECS_SOURCES})

# Configuration des répertoires d'inclusion
target_include_directories(ecs PUBLIC include)

# Installation
install(TARGETS ecs
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/ecs)