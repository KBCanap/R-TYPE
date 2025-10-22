cmake_minimum_required(VERSION 3.16)
project(MARIO VERSION 1.0.0 LANGUAGES CXX)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fichiers sources Mario
set(MARIO_SOURCES
    # Main entry point
    src/mario_main.cpp

    # Core game loop
    src/mario_game_core.cpp

    # Game systems (by functionality)
    src/mario_platforms.cpp
    src/mario_player.cpp
    src/mario_controls.cpp
    src/mario_render.cpp
    src/mario_animation.cpp
    src/mario_enemies.cpp

    # Shared app sources
    ../app/src/audio_manager.cpp
    ../app/src/render/RenderFactory.cpp
    ../app/src/render/sfml/SFMLRenderWindow.cpp
    ../app/src/render/sfml/SFMLRenderAudio.cpp
)

# Création de l'exécutable Mario
add_executable(r-type_mario ${MARIO_SOURCES})

# Configuration des répertoires d'inclusion
target_include_directories(r-type_mario PRIVATE
    include
    ../app/include
    ../ecs/include
)

# Lier avec les bibliothèques nécessaires
target_link_libraries(r-type_mario PRIVATE
    ecs
)

# Lier avec ASIO
if(TARGET asio_interface)
    target_link_libraries(r-type_mario PRIVATE asio_interface)
endif()

# Lier avec SFML
if(sfml-graphics_FOUND)
    target_link_libraries(r-type_mario PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        sfml-network
    )
else()
    target_link_libraries(r-type_mario PRIVATE
        ${SFML_LIBRARIES}
    )
    target_include_directories(r-type_mario PRIVATE ${SFML_INCLUDE_DIRS})
    target_compile_options(r-type_mario PRIVATE ${SFML_CFLAGS_OTHER})
endif()

# Fonction de configuration des warnings (définie dans le CMakeLists principal)
if(COMMAND setup_target_warnings)
    setup_target_warnings(r-type_mario)
endif()

# Configuration debug
target_compile_definitions(r-type_mario PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:RTYPE_DEBUG>
)

# Optimisations par configuration
target_compile_options(r-type_mario PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)
