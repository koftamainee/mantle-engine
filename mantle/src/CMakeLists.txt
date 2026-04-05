add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(renderer)

mantle_executable(NAME mantle MAIN main.cpp DEPS core window renderer)

set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/mantle/assets/shaders")

mantle_shader(mantle
        NAME triangle
        SOURCES "${SHADERS_DIR}/triangle.slang"
)