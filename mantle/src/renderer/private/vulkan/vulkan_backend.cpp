#include "vulkan_backend.h"
#include "vulkan/vulkan_utils.h"

#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "core/memory/scope_arena.h"
#include "vkassert.h"


namespace mantle {

    VulkanBackend::~VulkanBackend() { destroy(); }

    void VulkanBackend::init(const Window &window, bool vsync,
                             VirtualHeap *heap, ArenaAllocator *scratch_arena) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("vulkan").get();
        m_heap = heap;
        m_scratch_arena = scratch_arena;
        m_vsync = vsync;

        m_tlsf_allocator.init(m_heap->take(megabytes(100)));
        m_vk_allocator.init(&m_tlsf_allocator);

        m_context.init(window.get_native_window(), scratch_arena,
                       m_vk_allocator.vk_allocator());

        VkSurfaceKHR surface = m_context.get_surface();

        m_device.init(m_context.get_instance(), surface,
                      m_vk_allocator.vk_allocator(), m_heap, m_scratch_arena);
        VkDevice device = m_device.get_device();

        auto [width, height] = window.get_framebuffer_size();

        ScopeArena scope (m_scratch_arena);
        ArenaResource pmr(m_scratch_arena);

        m_swapchain.init(device, surface,
                         m_device.get_swapchain_support_details(surface, &pmr),
                         m_device.get_queue_families(), width, height, m_vsync,
                         m_vk_allocator.vk_allocator(), m_heap, m_scratch_arena);


        m_is_initialized = true;
        m_logger->info("Vulkan backend initialized");
    }

    void VulkanBackend::destroy() {
        if (m_is_initialized) {
            m_swapchain.destroy();
            m_device.destroy();
            m_context.destroy();
            m_is_initialized = false;

            m_logger->info("Vulkan backend destroyed");
        }
    }

    void VulkanBackend::wait_idle() const {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_VK_VERIFY(vkDeviceWaitIdle(m_device.get_device()));
    }

    void VulkanBackend::rebuild_swapchain(u32 width, u32 height) {
        MANTLE_CHECK(m_is_initialized);

        wait_idle();

        m_swapchain.destroy();

        VkDevice device = m_device.get_device();
        VkSurfaceKHR surface = m_context.get_surface();
        ScopeArena scope(m_scratch_arena);
        ArenaResource pmr(m_scratch_arena);
        m_swapchain.init(device, surface,
                         m_device.get_swapchain_support_details(surface, &pmr),
                         m_device.get_queue_families(), width, height, m_vsync,
                         m_vk_allocator.vk_allocator(), m_heap, m_scratch_arena);
    }

    SwapchainInfo VulkanBackend::get_swapchain_info() const {
        MANTLE_CHECK(m_is_initialized);

        auto [width, height] = m_swapchain.get_extent();
        return {
            .image_count = static_cast<u32>(m_swapchain.get_images().size()),
            .width = width,
            .height = height,
            .surface_format = from_vk(m_swapchain.get_surface_format().format),
            .depth_format = from_vk(m_device.get_supported_depth_format(false)),
        };
    }

    std::string_view VulkanBackend::gpu_name() const {
        return m_device.gpu_name();
    }

    u64 VulkanBackend::vram_bytes() const {
        return m_device.vram_bytes();
    }

    AcquiredImage
    VulkanBackend::acquire_next_image(VkSemaphore image_available) const {
        MANTLE_CHECK(m_is_initialized);

        u32 image_index = 0;
        VkResult result = vkAcquireNextImageKHR(
            m_device.get_device(), m_swapchain.get_swapchain(), UINT64_MAX,
            image_available, VK_NULL_HANDLE, &image_index);

        auto swapchain_result = SwapchainResult::Ok;
        if (result == VK_SUBOPTIMAL_KHR) {
            swapchain_result = SwapchainResult::Suboptimal;
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchain_result = SwapchainResult::OutOfDate;
        } else {
            MANTLE_VK_VERIFY(result);
        }

        return {.result = swapchain_result, .image_index = image_index};
    }

    SwapchainResult VulkanBackend::present(u32 image_index,
                                           VkSemaphore render_finished) const {
        VkSwapchainKHR swapchain = m_swapchain.get_swapchain();
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_finished,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index,
        };

        VkResult result =
            vkQueuePresentKHR(m_device.get_present_queue(), &present_info);

        auto swapchain_result = SwapchainResult::Ok;
        if (result == VK_SUBOPTIMAL_KHR) {
            swapchain_result = SwapchainResult::Suboptimal;
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchain_result = SwapchainResult::OutOfDate;
        } else {
            MANTLE_VK_VERIFY(result);
        }

        return swapchain_result;
    }
} // namespace mantle
