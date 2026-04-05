find_package(Vulkan REQUIRED)

mantle_module(NAME renderer DEPS Vulkan::Vulkan core window mesh camera)