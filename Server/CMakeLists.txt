cmake_minimum_required(VERSION 3.16)
project(RTYPE_SERVER VERSION 1.0.0 LANGUAGES CXX)

# ==== C++ Configuration ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==== Server Sources ====
set(SERVER_SOURCES
    # Core
    src/core/main.cpp
    src/core/StartServer.cpp

    # Game Logic
    src/gamelogic/GameSession.cpp
    src/gamelogic/GameLogic.cpp
    src/gamelogic/GameLogicPlayer.cpp
    src/gamelogic/GameLogicEntities.cpp
    src/gamelogic/GameLogicCombat.cpp
    src/gamelogic/GameLogicSystems.cpp
    src/gamelogic/GameLogicSnapshot.cpp

    # Server Loop
    src/serverloop/GameServerLoop.cpp

    # Network - TCP
    src/network/tcp/TcpServer.cpp

    # Network - UDP
    src/network/udp/UdpServer.cpp

    # Network - Protocol
    src/network/protocol/Protocole.cpp
    src/network/protocol/UdpProtocole.cpp

    # Network - Messages
    src/network/messages/ClientState.cpp
    src/network/messages/MessageQueue.cpp
    src/network/messages/MessageProcessor.cpp

    # ECS Registry (server needs it for game logic)
    ../ecs/src/registery.cpp
)

# ==== Server Executable ====
add_executable(r-type_server ${SERVER_SOURCES})

# ==== Include Directories ====
target_include_directories(r-type_server PRIVATE
    include
    ../ecs/include
)

# ==== Link Dependencies ====
# ASIO for networking
if(TARGET asio_interface)
    target_link_libraries(r-type_server PRIVATE asio_interface)
endif()

# Lua and Sol2 (for game logic scripts)
target_link_libraries(r-type_server PRIVATE
    lua
    sol2
)

# ==== Compiler Warnings ====
if(COMMAND setup_target_warnings)
    setup_target_warnings(r-type_server)
endif()

# ==== Compile Definitions ====
target_compile_definitions(r-type_server PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:RTYPE_DEBUG>
)

# ==== Optimizations ====
target_compile_options(r-type_server PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ==== Installation ====
install(TARGETS r-type_server
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)
