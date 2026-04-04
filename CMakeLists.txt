cmake_minimum_required(VERSION 3.26)
project(mantle CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    set(MANTLE_COMPILE_FLAGS /W4 /WX /GR- /EHs-)
else()
    set(MANTLE_COMPILE_FLAGS -fno-rtti -fno-exceptions)
endif()

function(mantle_module)
    cmake_parse_arguments(MOD "" "NAME" "DEPS" ${ARGN})

    file(GLOB_RECURSE MOD_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp"
    )
    file(GLOB_RECURSE MOD_PUBLIC_HEADERS
            "${CMAKE_CURRENT_SOURCE_DIR}/public/*.h"
    )

    if (NOT MOD_SOURCES)
        message(STATUS "[mantle_module] ${MOD_NAME} (interface)")
        add_library(${MOD_NAME} INTERFACE)
        target_include_directories(${MOD_NAME}
                INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/public"
        )
        if (MOD_DEPS)
            target_link_libraries(${MOD_NAME} INTERFACE ${MOD_DEPS})
        endif()
        return()
    endif()

    add_library(${MOD_NAME} STATIC ${MOD_SOURCES} ${MOD_PUBLIC_HEADERS})

    target_compile_options(${MOD_NAME} PRIVATE ${MANTLE_COMPILE_FLAGS})

    target_include_directories(${MOD_NAME}
            PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/public"
            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private"
    )

    if (MOD_DEPS)
        target_link_libraries(${MOD_NAME} PUBLIC ${MOD_DEPS})
    endif()

    message(STATUS "[mantle_module] ${MOD_NAME}")
endfunction()

function(mantle_executable)
    cmake_parse_arguments(EXE "" "NAME;MAIN" "DEPS" ${ARGN})

    if (NOT EXE_MAIN)
        message(FATAL_ERROR "mantle_executable(${EXE_NAME}): MAIN <file> is required")
    endif()

    add_executable(${EXE_NAME} ${EXE_MAIN})

    target_compile_options(${EXE_NAME} PRIVATE ${MANTLE_COMPILE_FLAGS})

    if (EXE_DEPS)
        target_link_libraries(${EXE_NAME} PRIVATE ${EXE_DEPS})
    endif()

    message(STATUS "[mantle_executable] ${EXE_NAME}")
endfunction()

add_subdirectory(mantle)