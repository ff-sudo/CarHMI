add_executable(carhmi_gui_tests
    test_insets.cpp
    test_size_policy.cpp
    test_box_layout.cpp
)

target_link_libraries(carhmi_gui_tests PRIVATE
    CarHMI::GUI
    GTest::gtest_main
    SDL2::SDL2-static
)

if(CARHMI_WARNING_FLAGS)
    target_compile_options(carhmi_gui_tests PRIVATE ${CARHMI_WARNING_FLAGS})
endif()

include(GoogleTest)
gtest_discover_tests(carhmi_gui_tests)
