add_executable(carhmi_core_tests
    test_connection.cpp
    test_event_bus.cpp
    test_property.cpp
    test_binding.cpp
    test_timer_manager.cpp
    test_resource_manager.cpp
    test_animation.cpp
)

target_link_libraries(carhmi_core_tests PRIVATE
    CarHMI::Core
    GTest::gtest_main
    SDL2::SDL2-static
)

if(CARHMI_WARNING_FLAGS)
    target_compile_options(carhmi_core_tests PRIVATE ${CARHMI_WARNING_FLAGS})
endif()

include(GoogleTest)
gtest_discover_tests(carhmi_core_tests)
