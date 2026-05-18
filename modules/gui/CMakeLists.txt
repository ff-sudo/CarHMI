carhmi_add_module(GUI)

target_link_libraries(carhmi_GUI
    PUBLIC
        CarHMI::Core
        CarHMI::RHI
        glm::glm
    PRIVATE
        spdlog::spdlog
        SDL2::SDL2-static
        sheenbidi
)

target_include_directories(carhmi_GUI PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party
)
