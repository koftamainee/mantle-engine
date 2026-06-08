function(mantle_module)
    cmake_parse_arguments(MOD "" "NAME" "DEPS" ${ARGN})

    if(NOT MOD_NAME)
        message(FATAL_ERROR "mantle_module: NAME is required")
    endif()

    file(GLOB_RECURSE MOD_SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp")
    file(GLOB_RECURSE MOD_HEADERS  "${CMAKE_CURRENT_SOURCE_DIR}/public/*.h")

    if(NOT MOD_SOURCES)
        message(STATUS "[mantle_module] ${MOD_NAME} (interface)")
        add_library(${MOD_NAME} INTERFACE)
        target_include_directories(${MOD_NAME}
                INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/public")
        if(MOD_DEPS)
            target_link_libraries(${MOD_NAME} INTERFACE ${MOD_DEPS})
        endif()
        return()
    endif()

    message(STATUS "[mantle_module] ${MOD_NAME}")
    add_library(${MOD_NAME} STATIC ${MOD_SOURCES} ${MOD_HEADERS})
    target_compile_options(${MOD_NAME} PRIVATE ${MANTLE_COMPILE_FLAGS})
    target_include_directories(${MOD_NAME}
            PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/public"
            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")
    if(MOD_DEPS)
        target_link_libraries(${MOD_NAME} PUBLIC ${MOD_DEPS})
    endif()
endfunction()
