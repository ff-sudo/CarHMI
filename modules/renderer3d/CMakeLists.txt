carhmi_add_module(Renderer3D)

target_link_libraries(carhmi_Renderer3D
    PUBLIC
        CarHMI::RHI
        glm::glm
    PRIVATE
        glad
        spdlog::spdlog
)

target_include_directories(carhmi_Renderer3D PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/tinyobjloader
    ${CMAKE_SOURCE_DIR}/third_party/stb
)
