cmake_minimum_required(VERSION 3.16)
project(MARIO VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

set(MARIO_SOURCES
    src/mario_main.cpp
    src/mario_game_core.cpp
    src/mario_platforms.cpp
    src/mario_player.cpp
    src/mario_controls.cpp
    src/mario_render.cpp
    src/mario_animation.cpp
    src/mario_enemies.cpp
    ../app/src/audio_manager.cpp
    ../app/src/render/RenderFactory.cpp
    ../app/src/render/sfml/SFMLRenderWindow.cpp
    ../app/src/render/sfml/SFMLRenderAudio.cpp
)

add_executable(r-type_mario ${MARIO_SOURCES})

set_target_properties(r-type_mario PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

target_include_directories(r-type_mario PRIVATE include ../app/include ../ecs/include)
target_link_libraries(r-type_mario PRIVATE ecs sfml_interface)
if(TARGET asio_interface)
    target_link_libraries(r-type_mario PRIVATE asio_interface)
endif()
