cmake_minimum_required(VERSION 3.16)
project(RTYPE_APP VERSION 1.0.0 LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fichiers sources de l'application
set(APP_SOURCES
    # Core
    src/core/main.cpp
    src/core/Game.cpp
    src/core/TickSystem.cpp

    # UI
    src/ui/Menu.cpp
    src/ui/GameMenu.cpp
    src/ui/ConnectionMenu.cpp
    src/ui/LobbyMenu.cpp
    src/ui/LobbyBrowserMenu.cpp
    src/ui/LevelSelectionMenu.cpp
    src/ui/OptionsMenu.cpp
    src/ui/PauseMenu.cpp
    src/ui/AccessibilityMenu.cpp
    src/ui/KeybindingsMenu.cpp
    src/ui/KeyBindings.cpp

    # Network
    src/network/ASIOSocket.cpp
    src/network/NetworkManager.cpp
    src/network/NetworkCommandHandler.cpp
    src/network/PacketProcessor.cpp

    # Game
    src/game/PlayerManager.cpp
    src/game/EnemyManager.cpp
    src/game/BossManager.cpp
    src/game/PowerupManager.cpp
    src/game/AudioManager.cpp

    # Render
    src/render/RenderFactory.cpp
    src/render/sfml/SFMLRenderWindow.cpp
    src/render/sfml/SFMLRenderAudio.cpp
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
    sfml_interface
    lua
    sol2
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