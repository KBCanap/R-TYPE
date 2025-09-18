# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

include(CTest)
include(Catch)

set(TEST_SOURCES
  example.test.cpp
  # add other tests here
)

add_executable(rtype_tests ${TEST_SOURCES})

target_link_libraries(rtype_tests PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(rtype_tests)
