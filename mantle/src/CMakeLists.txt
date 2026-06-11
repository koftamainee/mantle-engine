set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(core)
add_subdirectory(build_info)
add_subdirectory(system_info)
add_subdirectory(window)
add_subdirectory(renderer)
add_subdirectory(engine)
add_subdirectory(math)
add_subdirectory(noise)
add_subdirectory(ecs)
add_subdirectory(physics)
add_subdirectory(input)
add_subdirectory(camera)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mantle_init.cpp" "")
add_library(mantle SHARED "${CMAKE_CURRENT_BINARY_DIR}/mantle_init.cpp")
target_link_libraries(mantle PUBLIC
    -Wl,--whole-archive
    core build_info system_info window renderer engine math noise ecs physics input camera
    -Wl,--no-whole-archive
)
