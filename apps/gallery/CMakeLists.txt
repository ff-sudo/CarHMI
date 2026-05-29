add_executable(Gallery
    main.cpp
    application.cpp
    crash_handler.cpp
    dashboard_scene.cpp
    settings_scene.cpp
    widget_showcase_scene.cpp
    scene_3d_demo.cpp
    render_demo_scene.cpp
    layout_demo_scene.cpp
    timer_demo_scene.cpp
    canvas_demo_scene.cpp
    scroll_demo_scene.cpp
    textinput_demo_scene.cpp
    image_view_demo_scene.cpp
    dialog_demo_scene.cpp
)

target_link_libraries(Gallery PRIVATE
    CarHMI::GUI
    CarHMI::Core
    CarHMI::RHI
    CarHMI::PAL
    CarHMI::Renderer3D
    CarHMI::Runtime
    imgui
    glad
    spdlog::spdlog
    SDL2::SDL2-static
    SDL2::SDL2main
)

if(CARHMI_CRASH_HANDLER AND TARGET backward)
    target_compile_definitions(Gallery PRIVATE CARHMI_USE_BACKWARD)
    target_link_libraries(Gallery PRIVATE backward)
endif()

if(CARHMI_WARNING_FLAGS)
    target_compile_options(Gallery PRIVATE ${CARHMI_WARNING_FLAGS})
endif()

# Copy resources to build directory
add_custom_command(TARGET Gallery POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:Gallery>/resources
)
