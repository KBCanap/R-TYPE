cmake_minimum_required(VERSION 3.16)
project(libnet LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch standalone ASIO
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-28-1
)

FetchContent_MakeAvailable(asio)

# Build your dynamic library
add_library(net SHARED
    src/NetEntry.cpp
    src/NetServer.cpp
)

target_include_directories(net PRIVATE
    include
    ${asio_SOURCE_DIR}/asio/include
)

target_compile_features(net PRIVATE cxx_std_20)
