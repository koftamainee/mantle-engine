add_subdirectory(glm)

add_subdirectory(LuaJIT)

set(SOL2_BUILD_LUA OFF CACHE BOOL "" FORCE)
set(SOL2_TESTS OFF CACHE BOOL "" FORCE)
set(SOL2_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SOL2_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(sol2)

target_link_libraries(sol2 INTERFACE LuaJIT)

set(FLECS_SHARED OFF CACHE BOOL "" FORCE)
set(FLECS_STATIC ON CACHE BOOL "" FORCE)
set(FLECS_TESTS OFF CACHE BOOL "" FORCE)
set(FLECS_PIC ON CACHE BOOL "" FORCE)
add_subdirectory(flecs)
add_library(flecs ALIAS flecs_static)


set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(sdl)


set(SPDLOG_BUILD_EXAMPLE     OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS       OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH       OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL      OFF CACHE BOOL "" FORCE)
set(SPDLOG_NO_EXCEPTIONS     ON  CACHE BOOL "" FORCE)
add_subdirectory(spdlog)
target_compile_definitions(spdlog PUBLIC SPDLOG_NO_EXCEPTIONS)


set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)
add_subdirectory(googletest)


add_library(tlsf STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tlsf/tlsf.c)
target_include_directories(tlsf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tlsf)


set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

find_package(Vulkan REQUIRED)

target_link_libraries(imgui PUBLIC SDL3::SDL3-static Vulkan::Vulkan)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_USE_DYNAMIC_RENDERING=1)
