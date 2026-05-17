cmake_minimum_required(VERSION 3.20)
project(CarHMI VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# === SDL2 (local source) ===
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/SDL2)

# === GLM (local source, header-only) ===
add_subdirectory(third_party/glm)

# === spdlog (header-only) ===
add_subdirectory(third_party/spdlog)

# === GLAD (generated for OpenGL 3.3 Core) ===
add_library(glad STATIC
    third_party/glad/src/gl.c
)
target_include_directories(glad PUBLIC third_party/glad/include)

# === ImGui (local source) ===
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
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL2::SDL2-static glad)

# === SheenBidi (BiDi text support) ===
add_library(sheenbidi STATIC
    third_party/SheenBidi/Source/SheenBidi.c
)
target_include_directories(sheenbidi PUBLIC
    third_party/SheenBidi/Headers
)
target_include_directories(sheenbidi PRIVATE
    third_party/SheenBidi/Source
)
target_compile_definitions(sheenbidi PRIVATE SB_CONFIG_UNITY)

# === Main application ===
add_executable(CarHMI
    src/app/main.cpp
    src/app/dashboard_scene.cpp
    src/app/settings_scene.cpp
    src/app/widget_showcase_scene.cpp
    src/app/scene_3d_demo.cpp
    src/core/application.cpp
    src/platform/sdl2/sdl2_platform.cpp
    src/scene/scene_manager.cpp
    src/renderer/shader.cpp
    src/renderer/texture.cpp
    src/renderer/batch_renderer2d.cpp
    src/renderer/font.cpp
    src/renderer3d/mesh.cpp
    src/renderer3d/renderer3d.cpp
    src/gui/ui_context.cpp
    src/gui/widget.cpp
    src/gui/focus_manager.cpp
    src/i18n/bidi.cpp
)
target_include_directories(CarHMI PRIVATE
    src
    third_party/stb
    third_party/eventpp/include
    third_party/tinyobjloader
    third_party
)
target_link_libraries(CarHMI PRIVATE
    SDL2::SDL2-static
    SDL2::SDL2main
    glad
    imgui
    glm::glm
    spdlog::spdlog
    sheenbidi
)

# Copy resources to build directory
add_custom_command(TARGET CarHMI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:CarHMI>/resources
)
