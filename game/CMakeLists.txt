file(GLOB_RECURSE GAME_SOURCES "*.cpp")

add_executable(game ${GAME_SOURCES})
target_compile_options(game PRIVATE ${MANTLE_COMPILE_FLAGS})
target_link_libraries(game PRIVATE mantle)

add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -D SRC="${CMAKE_SOURCE_DIR}/mantle/assets"
    -D DST="${CMAKE_BINARY_DIR}/target/assets"
    -P "${CMAKE_SOURCE_DIR}/cmake/copy_assets.cmake"
    COMMENT "Copying assets"
    VERBATIM
)

add_error_codes(game)

set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/mantle/assets/shaders")

mantle_shader(game
    NAME mesh_render
    SOURCES "${SHADERS_DIR}/mesh_render.slang"
)

mantle_shader(game
    NAME basic_mesh
    SOURCES "${SHADERS_DIR}/basic_mesh.slang"
)
