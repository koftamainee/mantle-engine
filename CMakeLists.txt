cmake_minimum_required(VERSION 3.26)
project(mantle CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/target")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set_target_properties(${EXE_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/target"
)

foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${cfg} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/target")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/lib")
endforeach()

if (MSVC)
    set(MANTLE_COMPILE_FLAGS /W4 /WX /GR-)
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

function(mantle_shader TARGET)
    cmake_parse_arguments(SHD "" "NAME" "SOURCES" ${ARGN})

    if (NOT SHD_NAME)
        message(FATAL_ERROR "mantle_shader: NAME is required")
    endif()
    if (NOT SHD_SOURCES)
        message(FATAL_ERROR "mantle_shader: SOURCES is required")
    endif()

    if (NOT DEFINED SLANGC_EXECUTABLE)
        if (DEFINED ENV{VULKAN_SDK})
            set(SLANGC_EXECUTABLE "$ENV{VULKAN_SDK}/bin/slangc")
        else()
            message(FATAL_ERROR "VULKAN_SDK is not set and SLANGC_EXECUTABLE not provided")
        endif()
    endif()

    set(OUTPUT_SPV "${CMAKE_BINARY_DIR}/target/assets/shaders/${SHD_NAME}.spv")

    add_custom_command(
            OUTPUT  ${OUTPUT_SPV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/target/assets/shaders"
            COMMAND ${SLANGC_EXECUTABLE} ${SHD_SOURCES}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -entry vert_main -entry frag_main
            -o ${OUTPUT_SPV}
            DEPENDS ${SHD_SOURCES}
            COMMENT "Compiling shader ${SHD_NAME}"
            VERBATIM
    )

    add_custom_target(${TARGET}_shader_${SHD_NAME} DEPENDS ${OUTPUT_SPV})
    add_dependencies(${TARGET} ${TARGET}_shader_${SHD_NAME})
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

    set(ASSETS_SRC "${CMAKE_SOURCE_DIR}/mantle/assets")
    set(ASSETS_DST "${CMAKE_BINARY_DIR}/target/assets")

    add_custom_command(
            TARGET ${EXE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -D SRC="${CMAKE_SOURCE_DIR}/mantle/assets"
            -D DST="${CMAKE_BINARY_DIR}/target/assets"
            -P "${CMAKE_SOURCE_DIR}/cmake/copy_assets.cmake"
            COMMENT "Copying assets"
            VERBATIM
    )

    message(STATUS "[mantle_executable] ${EXE_NAME}")
endfunction()

add_subdirectory(mantle)