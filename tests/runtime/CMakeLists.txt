add_executable(carhmi_runtime_tests
    test_widget_factory.cpp
    test_scene_loader.cpp
    test_data_model.cpp
)

target_link_libraries(carhmi_runtime_tests PRIVATE
    CarHMI::Runtime
    CarHMI::GUI
    CarHMI::Core
    GTest::gtest_main
    spdlog::spdlog
    SDL2::SDL2-static
)

if(CARHMI_WARNING_FLAGS)
    target_compile_options(carhmi_runtime_tests PRIVATE ${CARHMI_WARNING_FLAGS})
endif()

include(GoogleTest)
gtest_discover_tests(carhmi_runtime_tests)
