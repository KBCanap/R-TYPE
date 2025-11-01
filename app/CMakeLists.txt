cmake_minimum_required(VERSION 3.16)
project(RTYPE_APP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

set(APP_SOURCES
    src/main.cpp
    src/menu.cpp
    src/game_menu.cpp
    src/audio_manager.cpp
    src/game.cpp
    src/tick_system.cpp
    src/options_menu.cpp
    src/accessibility_menu.cpp
    src/keybindings_menu.cpp
    src/key_bindings.cpp
    src/player_manager.cpp
    src/enemy_manager.cpp
    src/boss_manager.cpp
    src/powerup_manager.cpp
    src/pause_menu.cpp
    src/connection_menu.cpp
    src/lobby_browser_menu.cpp
    src/lobby_menu.cpp
    src/level_selection_menu.cpp
    src/render/RenderFactory.cpp
    src/render/sfml/SFMLRenderWindow.cpp
    src/render/sfml/SFMLRenderAudio.cpp
    src/ASIOSocket.cpp
    src/NetworkManager.cpp
    src/NetworkCommandHandler.cpp
    src/PacketProcessor.cpp
)

add_executable(r-type_app ${APP_SOURCES})

set_target_properties(r-type_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

target_include_directories(r-type_app PRIVATE include ../ecs/include)
target_link_libraries(r-type_app PRIVATE ecs sfml_interface lua sol2)
if(TARGET asio_interface)
    target_link_libraries(r-type_app PRIVATE asio_interface)
endif()

target_compile_definitions(r-type_app PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:RTYPE_DEBUG>
    $<$<CONFIG:Debug>:DEBUG_MODE>
)
