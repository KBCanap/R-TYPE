cmake_minimum_required(VERSION 3.20)
project(RTYPE LANGUAGES CXX)

# ==== Language & tools ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include(cmake/CPM.cmake)

# ==== Asio ====
CPMFindPackage(
  NAME asio
  VERSION 1.28.0
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-28-0
  FIND_PACKAGE_ARGUMENTS QUIET
  OPTIONS
    "ASIO_NO_DEPRECATED ON"
)

# Interface pour Asio
add_library(asio_interface INTERFACE)
if(asio_FOUND AND NOT asio_ADDED)
    # Asio système trouvé
    target_link_libraries(asio_interface INTERFACE asio::asio)
    message(STATUS "Using system Asio")
else()
    # Asio depuis CPM (cache ou téléchargement automatique)
    target_include_directories(asio_interface SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
    if(asio_ADDED)
        message(STATUS "Downloaded Asio and using from source")
    else()
        message(STATUS "Using cached Asio from ${CPM_SOURCE_CACHE}")
    endif()
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
    "SFML_USE_STATIC_STD_LIBS OFF"
    "BUILD_SHARED_LIBS ON"
)

# Interface pour SFML
add_library(sfml_interface INTERFACE)
if(SFML_FOUND AND NOT SFML_ADDED)
    # SFML système trouvé (pré-compilé)
    target_link_libraries(sfml_interface INTERFACE
      SFML::Graphics SFML::Window SFML::System SFML::Audio SFML::Network
    )
    message(STATUS "Using system SFML ${SFML_VERSION}")
else()
    # SFML depuis CPM (cache ou téléchargement automatique)
    target_link_libraries(sfml_interface INTERFACE
      sfml-graphics sfml-window sfml-system sfml-audio sfml-network
    )
    target_include_directories(sfml_interface INTERFACE ${SFML_SOURCE_DIR}/include)
    
    if(SFML_ADDED)
        message(STATUS "Downloaded SFML and compiling from source")
    else()
        message(STATUS "Using cached SFML from ${CPM_SOURCE_CACHE}")
    endif()
    
    # ==== System dependency check pour compilation SFML ====
    include(CheckIncludeFileCXX)
    set(MISSING_LIBS "")

    # Check for OpenAL
    find_package(OpenAL QUIET)
    if(NOT OPENAL_FOUND)
        list(APPEND MISSING_LIBS "OpenAL (audio)")
    endif()

    # Check for libsndfile
    find_path(SNDFILE_INCLUDE_DIR sndfile.h)
    find_library(SNDFILE_LIBRARY sndfile)
    if(NOT SNDFILE_INCLUDE_DIR OR NOT SNDFILE_LIBRARY)
        list(APPEND MISSING_LIBS "libsndfile (audio file decoding)")
    endif()

    # Check for X11 (Linux only)
    if(UNIX AND NOT APPLE)
        find_package(X11 QUIET)
        if(NOT X11_FOUND)
            list(APPEND MISSING_LIBS "X11 (window system)")
        endif()
    endif()

    # Show install help if missing - VOTRE GUIDE PRÉSERVÉ
    if(MISSING_LIBS)
        message(WARNING "Some system libraries required by SFML are missing:")
        foreach(lib IN LISTS MISSING_LIBS)
            message(STATUS "   - ${lib}")
        endforeach()
        message(STATUS "")
        message(STATUS "To install them:")
        if(WIN32)
            message(STATUS "On Windows (PowerShell, admin):")
            message(STATUS "cd C:\\")
            message(STATUS "git clone https://github.com/microsoft/vcpkg.git")
            message(STATUS "cd vcpkg")
            message(STATUS ".\\bootstrap-vcpkg.bat")
            message(STATUS ".\\vcpkg install sfml:x64-windows openal-soft:x64-windows libsndfile:x64-windows")
            message(STATUS "")
        elseif(UNIX)
            message(STATUS "On Linux (Ubuntu/Debian):")
            message(STATUS "     sudo apt install -y libsfml-dev libopenal-dev libsndfile1-dev libx11-dev")
            message(STATUS "")
        endif()
    endif()
endif()

# Common definitions
add_compile_definitions(ASIO_STANDALONE ASIO_NO_DEPRECATED)

# ==== Utility function for target configuration ====
function(setup_target_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /EHsc
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wold-style-cast
            -Woverloaded-virtual
            -Wnull-dereference
        )
    endif()

    if (UNIX)
        find_package(Threads REQUIRED)
        target_link_libraries(${target} PRIVATE Threads::Threads)
    endif()

    if (WARNINGS_AS_ERRORS)
        if (MSVC)
            target_compile_options(${target} PRIVATE /WX)
        else()
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# ==== Subprojects ====
# ECS Library
add_subdirectory(ecs)

# App (R-Type client with ECS)
add_subdirectory(app)

# ==== Main targets ====
# Server
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
target_include_directories(r-type_server PRIVATE
  Server/include
)
target_link_libraries(r-type_server PRIVATE
  asio_interface
)

# Warning and debug configuration
setup_target_warnings(r-type_server)
target_compile_definitions(r-type_server PRIVATE 
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:Debug>:RTYPE_DEBUG>
)

# Client is now handled by the app module

# ==== Unit tests (optional) ====
if(RTYPE_BUILD_TESTS)
  enable_testing()
  
  # Catch2 avec logique complète : système → cache → installation
  CPMFindPackage(
    NAME Catch2
    VERSION 3.5.3
    GITHUB_REPOSITORY catchorg/Catch2
    GIT_TAG v3.5.3
      FIND_PACKAGE_ARGUMENTS "CONFIG QUIET"
    OPTIONS
      "CATCH_INSTALL_DOCS OFF"
      "CATCH_INSTALL_EXTRAS ON"
  )

  if(Catch2_FOUND AND NOT Catch2_ADDED)
    message(STATUS "Using system Catch2")
  else()
    if(Catch2_ADDED)
      message(STATUS "Downloaded Catch2 and using from source")
    else()
      message(STATUS "Using cached Catch2 from ${CPM_SOURCE_CACHE}")
    endif()
  endif()
  
  if(Catch2_ADDED)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  endif()

  # Add your tests here
  add_subdirectory(tests)
  
  message(STATUS "Unit tests enabled")
endif()

# ==== Optimizations by configuration (fixed for multi-config) ====
# Avoid CMAKE_BUILD_TYPE here and use generator expressions:
# Release: -O3 -DNDEBUG only
target_compile_options(r-type_server PRIVATE
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ==== Installation (for packaging/deployment) ====
include(GNUInstallDirs)

install(TARGETS r-type_app r-type_server
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT Runtime
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "R-Type")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "R-Type multiplayer game")

# Linux-specific configuration
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgcc-s1, libstdc++6")

include(CPack)

# ==== Build information ====
message(STATUS "=== R-Type Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CPM cache: ${CPM_SOURCE_CACHE}")
message(STATUS "Warnings as errors: ${RTYPE_WERROR}")
message(STATUS "CI build: ${RTYPE_CI_BUILD}")
message(STATUS "Build tests: ${RTYPE_BUILD_TESTS}")
message(STATUS "===================================")