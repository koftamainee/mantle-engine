add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(renderer)
add_subdirectory(camera)
add_subdirectory(mesh)
add_subdirectory(world)
add_subdirectory(engine)

mantle_executable(NAME mantle MAIN main.cpp DEPS engine)

set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/mantle/assets/shaders")

mantle_shader(mantle
        NAME triangle
        SOURCES "${SHADERS_DIR}/triangle.slang"
)