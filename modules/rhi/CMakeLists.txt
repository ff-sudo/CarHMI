carhmi_add_module(RHI)

target_include_directories(carhmi_RHI PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/stb
)

target_link_libraries(carhmi_RHI
    PUBLIC
        glad
        glm::glm
    PRIVATE
        spdlog::spdlog
)
