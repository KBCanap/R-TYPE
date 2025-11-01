cmake_minimum_required(VERSION 3.16)
project(ECS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

set(ECS_SOURCES
    src/registery.cpp
    src/projectile_pattern.cpp
    src/ai_movement_pattern.cpp
    src/weapon.cpp
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
    src/NetworkSystem.cpp
    src/ANetworkManager.cpp
)

add_library(ecs STATIC ${ECS_SOURCES})

# ==== Sortie dans /lib ====
set_target_properties(ecs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set_target_properties(ecs PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/lib
    )
endforeach()

target_include_directories(ecs PUBLIC include)
target_link_libraries(ecs PUBLIC lua sol2)
if(TARGET asio_interface)
    target_link_libraries(ecs PUBLIC asio_interface)
endif()
