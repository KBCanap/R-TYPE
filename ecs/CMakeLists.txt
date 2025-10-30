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
    # Network sources
    src/NetworkSystem.cpp
    src/ANetworkManager.cpp
)

# Sur Windows, utilisez STATIC pour éviter les problèmes de DLL
# Sur Linux/Mac, SHARED fonctionne bien
if(WIN32)
    add_library(ecs STATIC ${ECS_SOURCES})
    # Pour les bibliothèques statiques sur Windows avec MSVC
    if(MSVC)
        set_target_properties(ecs PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/$<CONFIG>
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/Debug
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/Release
        )
    endif()
else()
    add_library(ecs SHARED ${ECS_SOURCES})
    set_target_properties(ecs PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )
endif()

# Configuration des répertoires d'inclusion
target_include_directories(ecs PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Lier avec ASIO pour les types dans les headers network
# PRIVATE car asio n'est pas exposé dans l'API publique d'ECS
if(TARGET asio_interface)
    target_link_libraries(ecs PRIVATE asio_interface)
endif()