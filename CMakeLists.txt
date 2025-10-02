cmake_minimum_required(VERSION 3.20)
project(RTYPE LANGUAGES CXX)

# ==== Langage & outils ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# include_directories(/home/dylanadghar/delivery/tek_3/r-type/asio-1.36.0/include) # Removed hardcoded path - using CPM instead

include(cmake/CPM.cmake)

# Asio (standalone, header-only)
CPMAddPackage(
  NAME asio
  VERSION 1.28.0
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-28-0
  OPTIONS
    "ASIO_NO_DEPRECATED ON"
)

# Recherche de SFML
find_package(sfml-graphics QUIET)
find_package(sfml-window QUIET)
find_package(sfml-system QUIET)
find_package(sfml-audio QUIET)
find_package(sfml-network QUIET)

# Si SFML n'est pas trouvé via find_package, essayer pkg-config
if(NOT sfml-graphics_FOUND OR NOT sfml-window_FOUND OR NOT sfml-system_FOUND OR NOT sfml-audio_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SFML REQUIRED sfml-graphics>=2.5 sfml-window>=2.5 sfml-system>=2.5 sfml-audio>=2.5 sfml-network>=2.5)
endif()

# Interface pour exposer proprement l'include Asio (SYSTEM pour éviter les warnings)
add_library(asio_interface INTERFACE)
if(asio_ADDED)
  target_include_directories(asio_interface SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
else()
  # Fallback si Asio est déjà installé sur le système
  find_package(asio REQUIRED)
  target_link_libraries(asio_interface INTERFACE asio::asio)
endif()

# Définitions communes
add_compile_definitions(ASIO_STANDALONE ASIO_NO_DEPRECATED)

# Configuration spécifique Windows
if(WIN32)
  # Cibler Windows 10+ (0x0A00)
  add_compile_definitions(_WIN32_WINNT=0x0A00)
  # Éviter les conflits avec les macros Windows
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# ==== Fonction utilitaire pour configuration des targets ====
function(setup_target_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE 
      /W4                   # Niveau d'avertissements élevé
      /w14640               # Thread unsafe static member initialization
      /w14826               # Conversion signedness warning
      /w14905               # Wide string literal cast
      /w14906               # String literal cast
    )
  else()
    target_compile_options(${target} PRIVATE 
      -Wall -Wextra -Wpedantic
      # Suppression des warnings stricts qui causent des problèmes avec ASIO
      # -Wconversion          # Conversions implicites (retiré)
      # -Wsign-conversion     # Conversions de signe (retiré)
      -Wold-style-cast      # Cast style C
      -Woverloaded-virtual  # Virtual function masking
      -Wnull-dereference    # Null pointer dereference
    )
    # Lien avec les threads sur Unix
    find_package(Threads REQUIRED)
    target_link_libraries(${target} PRIVATE Threads::Threads)
  endif()
endfunction()

# ==== Sous-projets ====
# ECS Library
add_subdirectory(ecs)

# App (R-Type client with ECS)
add_subdirectory(app)

# ==== Cibles principales ====
# Server
set(SERVER_SOURCES
    Server/src/main.cpp
    Server/src/StartServer.cpp
    Server/src/GameSession.cpp
    Server/src/ClientState.cpp
    Server/src/Protocole.cpp
    Server/src/Tcpserver.cpp
)

add_executable(r-type_server ${SERVER_SOURCES})
target_include_directories(r-type_server PRIVATE
  Server/include
)
target_link_libraries(r-type_server PRIVATE
  asio_interface
)

# Configuration warnings et debug
setup_target_warnings(r-type_server)
target_compile_definitions(r-type_server PRIVATE 
  $<$<CONFIG:Debug>:DEBUG>
  $<$<CONFIG:Debug>:RTYPE_DEBUG>
)

# Client is now handled by the app module

# ==== Tests unitaires (optionnel) ====
if(RTYPE_BUILD_TESTS)
  enable_testing()
  
  # Catch2 via CPM (v3)
  CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    VERSION 3.5.3
    OPTIONS
      "CATCH_INSTALL_DOCS OFF"
      "CATCH_INSTALL_EXTRAS ON" # pour include(Catch) et catch_discover_tests
  )
  if(Catch2_ADDED)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  endif()

  # Ici vous pourrez ajouter vos tests
  add_subdirectory(tests)
  
  message(STATUS "Unit tests enabled")
endif()

# ==== Optimisations par configuration (corrigé pour multi-config) ====
# On évite CMAKE_BUILD_TYPE ici et on utilise des generator expressions :
# - MSVC : /O2 seulement en Release
# - Autres : -O3 -DNDEBUG seulement en Release
target_compile_options(r-type_server PRIVATE
  $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2>
  $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:Release>>:-O3 -DNDEBUG>
)

# ==== Installation (pour packaging/déploiement) ====
include(GNUInstallDirs)

install(TARGETS r-type_app r-type_server
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT Runtime
)

# Configuration CPack pour packaging
set(CPACK_PACKAGE_NAME "R-Type")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "R-Type multiplayer game")
set(CPACK_PACKAGE_CONTACT "your-email@domain.com")

# Configuration spécifique par OS
if(WIN32)
  set(CPACK_GENERATOR "ZIP;NSIS")
else()
  set(CPACK_GENERATOR "TGZ;DEB")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgcc-s1, libstdc++6")
endif()

include(CPack)

# ==== Informations de build ====
message(STATUS "=== R-Type Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CPM cache: ${CPM_SOURCE_CACHE}")
message(STATUS "Warnings as errors: ${RTYPE_WERROR}")
message(STATUS "CI build: ${RTYPE_CI_BUILD}")
message(STATUS "Build tests: ${RTYPE_BUILD_TESTS}")
message(STATUS "===================================")