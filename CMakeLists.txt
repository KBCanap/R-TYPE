cmake_minimum_required(VERSION 3.20)
project(RTYPE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(/home/dylanadghar/delivery/tek_3/r-type/asio-1.36.0/include)

include(cmake/CPM.cmake)

# Asio (standalone version)
CPMAddPackage(
    NAME asio
    VERSION 1.28.0
    GITHUB_REPOSITORY "chriskohlhoff/asio"
    GIT_TAG "asio-1-28-0"
    OPTIONS "ASIO_USE_BOOST OFF"
)

# SFML
CPMAddPackage(
    NAME SFML
    VERSION 2.6.0
    GITHUB_REPOSITORY "SFML/SFML"
    GIT_TAG "2.6.0"
    OPTIONS
        "SFML_BUILD_AUDIO ON"
        "SFML_BUILD_GRAPHICS ON"
        "SFML_BUILD_WINDOW ON"
        "SFML_BUILD_SYSTEM ON"
        "SFML_BUILD_NETWORK ON"
        "SFML_USE_SYSTEM_DEPENDENCIES OFF"
)

# Server target
set(SERVER_SOURCES
    Server/src/main.cpp
)

add_executable(r-type_server ${SERVER_SOURCES})

target_include_directories(r-type_server PRIVATE
    Server/include
    ${asio_SOURCE_DIR}/asio/include
)

# Définitions nécessaires pour Asio
target_compile_definitions(r-type_server PRIVATE
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

# Client target
set(CLIENT_SOURCES
    Client/src/main.cpp
)

add_executable(r-type_client ${CLIENT_SOURCES})

target_include_directories(r-type_client PRIVATE
    Client/include
    ${asio_SOURCE_DIR}/asio/include
)

target_link_libraries(r-type_client PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)

# Définitions nécessaires pour Asio côté client aussi
target_compile_definitions(r-type_client PRIVATE
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

# Configuration spécifique par plateforme
if(MSVC)
    target_compile_options(r-type_client PRIVATE /W4)
    target_compile_options(r-type_server PRIVATE /W4)
    target_compile_definitions(r-type_server PRIVATE _WIN32_WINNT=0x0A00)
    target_compile_definitions(r-type_client PRIVATE _WIN32_WINNT=0x0A00)
else()
    target_compile_options(r-type_client PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(r-type_server PRIVATE -Wall -Wextra -Wpedantic)
    # Lier pthread sur Linux/Unix
    find_package(Threads REQUIRED)
    target_link_libraries(r-type_server PRIVATE Threads::Threads)
endif()

# Mode Debug/Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(r-type_server PRIVATE DEBUG)
    target_compile_definitions(r-type_client PRIVATE DEBUG)
endif()
