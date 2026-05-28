add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(renderer)
add_subdirectory(camera)
add_subdirectory(engine)
add_subdirectory(math)
add_subdirectory(world)
add_subdirectory(noise)

mantle_executable(NAME mantle MAIN main.cpp DEPS engine)

set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/mantle/assets/shaders")

mantle_shader(mantle
        NAME blit
        SOURCES "${SHADERS_DIR}/blit.slang"
)

mantle_shader(mantle
        NAME dda
        ENTRIES main
        SOURCES "${SHADERS_DIR}/dda.slang"
)