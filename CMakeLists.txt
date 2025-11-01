cmake_minimum_required(VERSION 3.20)
project(RTYPE LANGUAGES CXX)

# ==== Build global en statique ====
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# ==== Language & tools ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==== Répertoires de sortie ====
# Binaires exécutables à la racine
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
# Librairies dans /lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Support multi-config (Visual Studio)
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_SOURCE_DIR}/lib)
endforeach()

include(cmake/CPM.cmake)

# ==== Asio ====
CPMFindPackage(
  NAME asio
  VERSION 1.28.0
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-28-0
  FIND_PACKAGE_ARGUMENTS QUIET
  OPTIONS "ASIO_NO_DEPRECATED ON"
)

add_library(asio_interface INTERFACE)
if(asio_FOUND AND NOT asio_ADDED)
    target_link_libraries(asio_interface INTERFACE asio::asio)
else()
    target_include_directories(asio_interface SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
endif()

# ==== SFML ====
CPMFindPackage(
  NAME SFML
  VERSION 2.6.1
  GITHUB_REPOSITORY SFML/SFML
  GIT_TAG 2.6.1
  FIND_PACKAGE_ARGUMENTS "COMPONENTS graphics window system audio network QUIET"
  OPTIONS
    "SFML_BUILD_EXAMPLES OFF"
    "SFML_BUILD_DOC OFF"
    "SFML_USE_STATIC_STD_LIBS ON"
    "BUILD_SHARED_LIBS OFF"
)

add_library(sfml_interface INTERFACE)
if(SFML_FOUND AND NOT SFML_ADDED)
    target_link_libraries(sfml_interface INTERFACE
        SFML::Graphics SFML::Window SFML::System SFML::Audio SFML::Network
    )
    target_compile_definitions(sfml_interface INTERFACE SFML_STATIC)
else()
    target_link_libraries(sfml_interface INTERFACE
        sfml-graphics sfml-window sfml-system sfml-audio sfml-network
    )
    target_include_directories(sfml_interface INTERFACE ${SFML_SOURCE_DIR}/include)
    target_compile_definitions(sfml_interface INTERFACE SFML_STATIC)
endif()

# ==== Lua ====
find_path(LUA_INCLUDE_DIR lua.h PATHS /usr/include /usr/local/include)
find_library(LUA_LIBRARY lua PATHS /usr/lib /usr/lib64 /usr/local/lib)

if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIBRARY)
    message(FATAL_ERROR "Lua not found. Please install lua and lua-devel.")
endif()

add_library(lua STATIC IMPORTED GLOBAL)
set_target_properties(lua PROPERTIES
  IMPORTED_LOCATION ${LUA_LIBRARY}
  INTERFACE_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR}
)

# ==== Sol2 ====
CPMFindPackage(
  NAME sol2
  VERSION 3.3.0
  GITHUB_REPOSITORY ThePhD/sol2
  GIT_TAG v3.3.0
  OPTIONS "SOL2_EXAMPLES OFF"
)

# ==== Subprojects ====
add_subdirectory(ecs)
add_subdirectory(app)
add_subdirectory(mario)

# ==== Serveur ====
set(SERVER_SOURCES
    Server/src/main.cpp
    Server/src/StartServer.cpp
    Server/src/GameSession.cpp
    Server/src/ClientState.cpp
    Server/src/Protocole.cpp
    Server/src/Tcpserver.cpp
    Server/src/UdpServer.cpp
    Server/src/MessageQueue.cpp
    Server/src/GameServerLoop.cpp
    Server/src/UdpProtocole.cpp
    Server/src/GameLogic.cpp
    ecs/src/registery.cpp
)

add_executable(r-type_server ${SERVER_SOURCES})
target_include_directories(r-type_server PRIVATE Server/include)
target_link_libraries(r-type_server PRIVATE asio_interface)

# ==== Compilation options ====
if(MSVC)
    target_compile_options(r-type_server PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(r-type_server PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(r-type_server PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:RTYPE_DEBUG>
)

target_compile_options(r-type_server PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ==== Installation ====
include(GNUInstallDirs)
install(TARGETS r-type_app r-type_server r-type_mario
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
