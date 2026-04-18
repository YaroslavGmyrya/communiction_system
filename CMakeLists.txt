cmake_minimum_required(VERSION 3.16)

project(MyProject LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
pkg_check_modules(FFTW REQUIRED IMPORTED_TARGET fftw3)

include(FetchContent)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(IMGUI_DEP ${PROJECT_SOURCE_DIR}/third_party/imgui)
set(IMPLOT_DEP ${PROJECT_SOURCE_DIR}/third_party/implot)

set(IMGUI
    ${IMGUI_DEP}/imgui.cpp
    ${IMGUI_DEP}/imgui_draw.cpp
    ${IMGUI_DEP}/imgui_tables.cpp
    ${IMGUI_DEP}/imgui_widgets.cpp
    ${IMGUI_DEP}/imgui_demo.cpp
)

set(IMPLOT
    ${IMPLOT_DEP}/implot.cpp
    ${IMPLOT_DEP}/implot_items.cpp
    ${IMPLOT_DEP}/implot_demo.cpp
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
)
FetchContent_MakeAvailable(spdlog)

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
)

target_include_directories(CORE PUBLIC
    ${INCLUDE_DIR}
)

target_link_libraries(CORE PUBLIC
    fmt::fmt
    PkgConfig::FFTW
)

if(APPLE)

    find_package(glfw3 REQUIRED)

    add_executable(main
        src/main.cpp
        ${IMGUI}
        ${IMPLOT}
        third_party/backend/glfw_adapter.mm
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_metal.mm
        visualization/macOS.mm
    )

    target_include_directories(CORE PUBLIC
        ${INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party
        ${PROJECT_SOURCE_DIR}/third_party/imgui
        ${PROJECT_SOURCE_DIR}/third_party/imgui/backends
        ${PROJECT_SOURCE_DIR}/third_party/implot
    )

    target_link_libraries(main PRIVATE
        CORE
        spdlog::spdlog
        glfw
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework AppKit"
        objc
    )

elseif(UNIX)

    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)

    add_executable(main
        src/main.cpp
        ${IMGUI}
        ${IMPLOT}
        third_party/imgui/backends/imgui_impl_sdl2.cpp
        third_party/imgui/backends/imgui_impl_opengl3.cpp
    )

    target_include_directories(CORE PUBLIC
        ${INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party/imgui
        ${PROJECT_SOURCE_DIR}/third_party/imgui/backends
        ${PROJECT_SOURCE_DIR}/third_party/implot
    )

    target_link_libraries(main PRIVATE
        CORE
        spdlog::spdlog
        SDL2::SDL2
        OpenGL::GL
    )

endif()