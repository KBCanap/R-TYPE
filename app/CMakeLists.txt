cmake_minimum_required(VERSION 3.16)
project(RTYPE_APP VERSION 1.0.0 LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fichiers sources de l'application
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

# Création de l'exécutable client
add_executable(r-type_app ${APP_SOURCES})

# Configuration des répertoires d'inclusion
target_include_directories(r-type_app PRIVATE
    include
    ../ecs/include
)

# Lier avec les bibliothèques nécessaires
target_link_libraries(r-type_app PRIVATE
    ecs
)

# Lier avec ASIO pour le NetworkManager
if(TARGET asio_interface)
    target_link_libraries(r-type_app PRIVATE asio_interface)
endif()

# Lier avec SFML
if(sfml-graphics_FOUND)
    target_link_libraries(r-type_app PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        sfml-network
    )
else()
    # Utiliser pkg-config
    target_link_libraries(r-type_app PRIVATE
        ${SFML_LIBRARIES}
    )
    target_include_directories(r-type_app PRIVATE ${SFML_INCLUDE_DIRS})
    target_compile_options(r-type_app PRIVATE ${SFML_CFLAGS_OTHER})
endif()

# Fonction de configuration des warnings (définie dans le CMakeLists principal)
if(COMMAND setup_target_warnings)
    setup_target_warnings(r-type_app)
endif()

# Configuration debug
target_compile_definitions(r-type_app PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:RTYPE_DEBUG>
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# Optimisations par configuration
target_compile_options(r-type_app PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)