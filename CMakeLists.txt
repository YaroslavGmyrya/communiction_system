cmake_minimum_required(VERSION 3.16)

project(MyProject CXX)

set(CMAKE_BUILD_TYPE Debug)

#install google test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

#install spdlog (logger)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # for Windows

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(spdlog)

add_executable(main src/main.cpp)
add_executable(tests tests/tests.cpp)

target_link_libraries(main spdlog::spdlog)
target_link_libraries(tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)



