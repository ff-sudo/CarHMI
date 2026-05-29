carhmi_add_module(Runtime)

target_link_libraries(carhmi_Runtime
    PUBLIC
        CarHMI::Core
        CarHMI::GUI
        glm::glm
    PRIVATE
        spdlog::spdlog
        SDL2::SDL2-static
)

target_include_directories(carhmi_Runtime PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party
)
