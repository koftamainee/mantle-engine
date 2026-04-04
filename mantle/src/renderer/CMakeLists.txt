find_package(Vulkan REQUIRED COMPONENTS volk)

mantle_module(NAME renderer DEPS Vulkan::Vulkan Vulkan::volk core window)