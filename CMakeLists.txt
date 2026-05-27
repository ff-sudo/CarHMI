cmake_minimum_required(VERSION 3.20)
project(CarHMI VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warning flags (applied only to CarHMI targets via module macro)
if(MSVC)
    set(CARHMI_WARNING_FLAGS /W4 /wd4100 /wd4244 /wd4267)
else()
    set(CARHMI_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_definitions(-D__PRFCHWINTRIN_H)
endif()

# === CMake utilities ===
include(cmake/CarHMIModule.cmake)

# === Third-party dependencies ===

# SDL2
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/SDL2)

# GLM
add_subdirectory(third_party/glm)

# spdlog
add_subdirectory(third_party/spdlog)

# GLAD
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL2::SDL2-static glad)

# SheenBidi
add_library(sheenbidi STATIC third_party/SheenBidi/Source/SheenBidi.c)
target_include_directories(sheenbidi PUBLIC third_party/SheenBidi/Headers)
target_include_directories(sheenbidi PRIVATE third_party/SheenBidi/Source)
target_compile_definitions(sheenbidi PRIVATE SB_CONFIG_UNITY)

# === Modules (order matters for dependencies) ===
add_subdirectory(modules/core)
add_subdirectory(modules/pal)
add_subdirectory(modules/rhi)
add_subdirectory(modules/gui)
add_subdirectory(modules/renderer3d)

# Placeholder modules
add_subdirectory(modules/runtime)
add_subdirectory(modules/script)
add_subdirectory(modules/pipeline)

# === Applications ===
add_subdirectory(apps/gallery)

# === Tests ===
option(CARHMI_BUILD_TESTS "Build unit tests" ON)
if(CARHMI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
