add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(renderer)
add_subdirectory(camera)
add_subdirectory(engine)
add_subdirectory(math)

mantle_executable(NAME mantle MAIN main.cpp DEPS engine)

set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/mantle/assets/shaders")

mantle_shader(mantle
        NAME flat
        SOURCES "${SHADERS_DIR}/flat.slang"
)