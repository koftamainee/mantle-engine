cmake_minimum_required(VERSION 3.26)
project(mantle C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${cfg} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/target")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_BINARY_DIR}/lib")
endforeach()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/target")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(MSVC)
    set(MANTLE_COMPILE_FLAGS /W4 /GR-)
else()
    set(MANTLE_COMPILE_FLAGS -fno-rtti -fno-exceptions)
endif()

add_compile_definitions(
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RIGHT_HANDED

        MANTLE_DEBUG
)

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

function(mantle_shader TARGET)
    cmake_parse_arguments(SHD "" "NAME;ENTRIES" "SOURCES" ${ARGN})

    if(NOT SHD_NAME)
        message(FATAL_ERROR "mantle_shader: NAME is required")
    endif()
    if(NOT SHD_SOURCES)
        message(FATAL_ERROR "mantle_shader: SOURCES is required")
    endif()
    if(NOT SHD_ENTRIES)
        set(SHD_ENTRIES "vert_main;frag_main")
    endif()

    if(NOT DEFINED SLANGC_EXECUTABLE)
        if(DEFINED ENV{VULKAN_SDK})
                find_program(SLANGC_EXECUTABLE
        NAMES slangc slangc.exe
        HINTS
            $ENV{VULKAN_SDK}/Bin
            $ENV{VULKAN_SDK}/bin
    )
        else()
            message(FATAL_ERROR "VULKAN_SDK is not set and SLANGC_EXECUTABLE not provided")
        endif()
    endif()
    if(NOT EXISTS "${SLANGC_EXECUTABLE}")
        message(FATAL_ERROR "slangc not found at: ${SLANGC_EXECUTABLE}")
    endif()

    set(SHD_SOURCES_ABS)
    foreach(src IN LISTS SHD_SOURCES)
        get_filename_component(src_abs "${src}" ABSOLUTE)
        list(APPEND SHD_SOURCES_ABS "${src_abs}")
    endforeach()

    set(OUTPUT_SPV "${CMAKE_BINARY_DIR}/target/assets/shaders/${SHD_NAME}.spv")

    set(SHD_ENTRY_FLAGS)
    foreach(entry IN LISTS SHD_ENTRIES)
        list(APPEND SHD_ENTRY_FLAGS "-entry" "${entry}")
    endforeach()

    add_custom_command(
            OUTPUT ${OUTPUT_SPV}
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_BINARY_DIR}/target/assets/shaders"
            COMMAND ${SLANGC_EXECUTABLE} ${SHD_SOURCES_ABS}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            ${SHD_ENTRY_FLAGS}
            -o ${OUTPUT_SPV}
            DEPENDS ${SHD_SOURCES_ABS}
            COMMENT "Compiling shader ${SHD_NAME}"
            VERBATIM
    )

    add_custom_target(${TARGET}_shader_${SHD_NAME} DEPENDS ${OUTPUT_SPV})
    add_dependencies(${TARGET} ${TARGET}_shader_${SHD_NAME})
endfunction()

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

    if(UNIX)
        target_link_options(${EXE_NAME} PRIVATE "-Wl,-rpath,$ORIGIN/lib")
    endif()

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

    if(DEFINED ENV{VULKAN_SDK})
        if(UNIX)
            add_custom_command(TARGET ${EXE_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/target/lib"
                COMMAND ${CMAKE_COMMAND} -E copy "$ENV{VULKAN_SDK}/lib/libvulkan.so.1" "${CMAKE_BINARY_DIR}/target/lib/libvulkan.so.1"
                COMMENT "Copying Vulkan loader (libvulkan.so.1)"
            )
        elseif(WIN32)
            add_custom_command(TARGET ${EXE_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/target/lib"
                COMMAND ${CMAKE_COMMAND} -E copy "$ENV{VULKAN_SDK}/Bin/vulkan-1.dll" "${CMAKE_BINARY_DIR}/target/lib/"
                COMMENT "Copying Vulkan loader (vulkan-1.dll)"
            )
        endif()
    endif()
endfunction()

enable_testing()
add_subdirectory(mantle)