cmake_minimum_required(VERSION 3.26)
include(cmake/compiler.cmake)

project(mantle C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/output_dirs.cmake)
include(cmake/compile_flags.cmake)
include(cmake/tooling.cmake)

execute_process(COMMAND git -C "${CMAKE_SOURCE_DIR}" describe --always --dirty
    OUTPUT_VARIABLE MANTLE_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" MANTLE_VERSION_CONTENT
     OFFSET 0 LIMIT 64)
string(STRIP "${MANTLE_VERSION_CONTENT}" MANTLE_VERSION)

string(TIMESTAMP MANTLE_BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)

if(MSVC)
    set(MANTLE_COMPILER "MSVC ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(MANTLE_COMPILER "Clang ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(MANTLE_COMPILER "GCC ${CMAKE_CXX_COMPILER_VERSION}")
else()
    set(MANTLE_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endif()

add_compile_definitions(
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RIGHT_HANDED
        MANTLE_GIT_HASH="${MANTLE_GIT_HASH}"
        MANTLE_VERSION="${MANTLE_VERSION}"
        MANTLE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        MANTLE_BUILD_DATE="${MANTLE_BUILD_DATE}"
        MANTLE_COMPILER="${MANTLE_COMPILER}"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MANTLE_DEBUG)
endif()

include(cmake/mantle_module.cmake)

include(cmake/mantle_shader.cmake)

include(cmake/mantle_executable.cmake)
include(cmake/gen_error_codes.cmake)

enable_testing()
add_subdirectory(mantle)