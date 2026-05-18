carhmi_add_module(PAL)

target_link_libraries(carhmi_PAL
    PUBLIC
        glm::glm
    PRIVATE
        SDL2::SDL2-static
        glad
        spdlog::spdlog
        CarHMI::Core
)
