#include "vulkan_backend.h"

// TODO
namespace mantle {
    struct VulkanBackend::InternalAccess {};

    VulkanBackend::~VulkanBackend() {}

    void VulkanBackend::init(const Window &window, bool vsync,
                             VirtualHeap *heap, ArenaAllocator *scratch_arena) {
    }

    void VulkanBackend::destroy() {}

    void VulkanBackend::wait_idle() const {}

    void VulkanBackend::rebuild_swapchain(u32 width, u32 height) {}

    SwapchainInfo VulkanBackend::get_swapchain_info() const {}

    AcquiredImage VulkanBackend::acquire_next_image(u32 semaphore_index) const {
    }

    void VulkanBackend::present(u32 image_index, u32 semaphore_index) const {}

    VulkanBackend::InternalAccess VulkanBackend::get_internal() const {}
} // namespace mantle
