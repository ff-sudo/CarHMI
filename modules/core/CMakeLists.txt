carhmi_add_module(Core)

target_link_libraries(carhmi_Core
    PUBLIC
        glm::glm
        spdlog::spdlog
    PRIVATE
        SDL2::SDL2-static
)

target_include_directories(carhmi_Core PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/eventpp/include
)
