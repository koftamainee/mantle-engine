function(mantle_executable)
    cmake_parse_arguments(EXE "" "NAME;MAIN" "DEPS" ${ARGN})

    if(NOT EXE_NAME)
        message(FATAL_ERROR "mantle_executable: NAME is required")
    endif()
    if(NOT EXE_MAIN)
        message(FATAL_ERROR "mantle_executable(${EXE_NAME}): MAIN is required")
    endif()

    message(STATUS "[mantle_executable] ${EXE_NAME}")
    add_executable(${EXE_NAME} ${EXE_MAIN})
    target_compile_options(${EXE_NAME} PRIVATE ${MANTLE_COMPILE_FLAGS})

    if(EXE_DEPS)
        target_link_libraries(${EXE_NAME} PRIVATE ${EXE_DEPS})
    endif()

    add_custom_command(
            TARGET ${EXE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -D SRC="${CMAKE_SOURCE_DIR}/mantle/assets"
            -D DST="${CMAKE_BINARY_DIR}/target/assets"
            -P "${CMAKE_SOURCE_DIR}/cmake/copy_assets.cmake"
            COMMENT "Copying assets"
            VERBATIM
    )

endfunction()
