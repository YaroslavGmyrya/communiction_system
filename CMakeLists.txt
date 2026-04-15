cmake_minimum_required(VERSION 3.16)

project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED IMPORTED_TARGET fftw3)

include(FetchContent)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
)

FetchContent_MakeAvailable(spdlog)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)


set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(SRC_DIR         ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR     ${PROJECT_SOURCE_DIR}/include)
set(VIS_DIR         ${PROJECT_SOURCE_DIR}/visualization)


add_library(imgui STATIC
    ${THIRD_PARTY_DIR}/imgui/imgui.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/imgui/backends
)

target_link_libraries(imgui PUBLIC
    SDL2::SDL2
    OpenGL::GL
    GLEW::GLEW
)

set_target_properties(imgui PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)


add_library(implot STATIC
    ${THIRD_PARTY_DIR}/implot/implot.cpp
    ${THIRD_PARTY_DIR}/implot/implot_demo.cpp
    ${THIRD_PARTY_DIR}/implot/implot_items.cpp
)

target_include_directories(implot PUBLIC
    ${THIRD_PARTY_DIR}/implot
)

target_link_libraries(implot PUBLIC imgui)

set_target_properties(implot PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)


add_library(CORE SHARED
    src/tx_lib/modulator.cpp
    src/tx_lib/preparing_bits.cpp
    src/tx_lib/tx_run.cpp
    src/tx_lib/OFDM_tx.cpp

    src/rx_lib/decoder.cpp
    src/rx_lib/deintervaler.cpp
    src/rx_lib/demodulator.cpp
    src/rx_lib/find_peaks.cpp
    src/rx_lib/OFDM_rx.cpp
    src/rx_lib/rx_run.cpp

    src/channel_lib/AWGN.cpp

    visualization/ImGUI_interface.cpp
)

target_include_directories(CORE PUBLIC
    ${INCLUDE_DIR}
)

target_link_libraries(CORE PUBLIC
    imgui
    implot
    fmt::fmt

)


add_executable(main
    ${SRC_DIR}/main.cpp
)

target_link_libraries(main PRIVATE
    CORE
    spdlog::spdlog
    PkgConfig::FFTW

)
