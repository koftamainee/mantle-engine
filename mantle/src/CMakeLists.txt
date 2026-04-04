add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(renderer)

mantle_executable(NAME mantle MAIN main.cpp DEPS core window renderer)
