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

find_package(fmt REQUIRED)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # for Windows

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# OpenGL/GLEW
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${INCLUDE_DIR})

add_library(imgui STATIC
    ${THIRD_PARTY_DIR}/imgui/imgui.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC ${THIRD_PARTY_DIR}/imgui)
target_compile_options(imgui PRIVATE -fPIC)

add_library(implot STATIC
    ${THIRD_PARTY_DIR}/implot/implot.cpp
    ${THIRD_PARTY_DIR}/implot/implot_demo.cpp
    ${THIRD_PARTY_DIR}/implot/implot_items.cpp
)

target_include_directories(implot PUBLIC ${THIRD_PARTY_DIR}/implot)
target_link_libraries(implot PUBLIC imgui)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(spdlog)

add_library(CORE SHARED
  src/rx_lib/decoder.cpp
  src/tx_lib/coder.cpp
  src/tx_lib/conv_coder.cpp
  src/tx_lib/intervaler.cpp
  src/rx_lib/deintervaler.cpp
  src/tx_lib/modulator.cpp
  src/rx_lib/demodulator.cpp
)

add_executable(main src/main.cpp)
add_executable(tests tests/tests.cpp)

target_link_libraries(main PRIVATE spdlog::spdlog CORE fmt::fmt imgui implot ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
target_link_libraries(tests PRIVATE GTest::gtest_main CORE fmt::fmt)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)



