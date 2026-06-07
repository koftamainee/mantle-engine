add_subdirectory(core)
add_subdirectory(build_info)
add_subdirectory(system_info)
add_subdirectory(window)
add_subdirectory(renderer)
add_subdirectory(engine)
add_subdirectory(math)
add_subdirectory(world)
add_subdirectory(noise)
add_subdirectory(ecs)
add_subdirectory(lua)

mantle_executable(NAME mantle MAIN main.cpp DEPS engine)

set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/mantle/assets/shaders")

mantle_shader(mantle
        NAME mesh_render
        SOURCES "${SHADERS_DIR}/mesh_render.slang"
)