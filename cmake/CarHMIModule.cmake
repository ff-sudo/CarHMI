function(carhmi_add_module MODULE_NAME)
    set(options HEADER_ONLY)
    set(oneValueArgs "")
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(MOD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(TARGET_NAME carhmi_${MODULE_NAME})

    if(MOD_HEADER_ONLY)
        add_library(${TARGET_NAME} INTERFACE)
        target_include_directories(${TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
    else()
        if(MOD_SOURCES)
            add_library(${TARGET_NAME} STATIC ${MOD_SOURCES})
        else()
            file(GLOB_RECURSE _SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
            )
            add_library(${TARGET_NAME} STATIC ${_SOURCES})
        endif()

        target_include_directories(${TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
        target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        # Apply project warning flags
        if(CARHMI_WARNING_FLAGS)
            target_compile_options(${TARGET_NAME} PRIVATE ${CARHMI_WARNING_FLAGS})
        endif()
    endif()

    add_library(CarHMI::${MODULE_NAME} ALIAS ${TARGET_NAME})
endfunction()

function(carhmi_add_placeholder MODULE_NAME)
    set(TARGET_NAME carhmi_${MODULE_NAME})
    add_library(${TARGET_NAME} INTERFACE)
    target_include_directories(${TARGET_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    add_library(CarHMI::${MODULE_NAME} ALIAS ${TARGET_NAME})
endfunction()
