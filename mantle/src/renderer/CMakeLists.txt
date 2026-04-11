find_package(Vulkan REQUIRED)

mantle_module(NAME renderer DEPS Vulkan::Vulkan core window camera imgui)

target_include_directories(renderer PUBLIC $ENV{VULKAN_SDK}/include)