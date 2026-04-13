#include "vulkan_backend.h"

#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "vkassert.h"


namespace mantle {
    namespace {
        ImageFormat from_vk(VkFormat format) {
            switch (format) {
            case VK_FORMAT_R8G8B8A8_UNORM:
                return ImageFormat::Rgba8;
            case VK_FORMAT_R8G8B8A8_SRGB:
                return ImageFormat::Rgba8Srgb;
            case VK_FORMAT_R16G16_UNORM:
                return ImageFormat::Rg16;
            case VK_FORMAT_R32_SFLOAT:
                return ImageFormat::R32;
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return ImageFormat::D32S8;
            case VK_FORMAT_D32_SFLOAT:
                return ImageFormat::D32;
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
                return ImageFormat::D24S8;
            case VK_FORMAT_D16_UNORM:
                return ImageFormat::D16;
            case VK_FORMAT_B8G8R8A8_UNORM:
                return ImageFormat::Bgra8;
            case VK_FORMAT_B8G8R8A8_SRGB:
                return ImageFormat::Bgra8Srgb;
            default:
                checkf(false, "unsupported VkFormat");
            }
        };
    } // namespace

    VulkanBackend::~VulkanBackend() { destroy(); }

    void VulkanBackend::init(const Window &window, bool vsync,
                             VirtualHeap *heap, ArenaAllocator *scratch_arena) {
        check(!m_is_initialized);

        m_heap = heap;
        m_scratch_arena = scratch_arena;
        m_vsync = vsync;

        m_tlsf_allocator.init(m_heap->take(megabytes(100)));
        m_vk_allocator.init(&m_tlsf_allocator);

        m_context.init(window.get_native_window(), scratch_arena,
                       m_vk_allocator.vk_allocator());

        VkSurfaceKHR surface = m_context.get_surface();
        VkInstance instance = m_context.get_instance();

        m_device.init(m_context.get_instance(), surface,
                      m_vk_allocator.vk_allocator());
        VkDevice device = m_device.get_device();

        auto [width, height] = window.get_framebuffer_size();


        m_swapchain.init(device, surface,
                         m_device.get_swapchain_support_details(surface),
                         m_device.get_queue_families(), width, height, m_vsync,
                         m_vk_allocator.vk_allocator());

        m_gpu_allocator.init(m_device.get_physical_device(), device, instance,
                             m_vk_allocator.vk_allocator());

        m_is_initialized = true;
        spdlog::info("Vulkan Backend is initialized");
    }

    void VulkanBackend::destroy() {
        if (m_is_initialized) {
            m_gpu_allocator.destroy();
            m_swapchain.destroy();
            m_device.destroy();
            m_context.destroy();
            m_is_initialized = false;

            spdlog::info("Vulkan Backend is destroyed");
        }
    }

    void VulkanBackend::wait_idle() const {
        check(m_is_initialized);
        vk_verify(vkDeviceWaitIdle(m_device.get_device()));
    }

    void VulkanBackend::rebuild_swapchain(u32 width, u32 height) {
        check(m_is_initialized);

        wait_idle();

        m_swapchain.destroy();

        VkDevice device = m_device.get_device();
        VkSurfaceKHR surface = m_context.get_surface();
        m_swapchain.init(device, surface,
                         m_device.get_swapchain_support_details(surface),
                         m_device.get_queue_families(), width, height, m_vsync,
                         m_vk_allocator.vk_allocator());
    }

    SwapchainInfo VulkanBackend::get_swapchain_info() const {
        check(m_is_initialized);

        auto [width, height] = m_swapchain.get_extent();
        return {
            .image_count = static_cast<u32>(m_swapchain.get_images().size()),
            .width = width,
            .height = height,
            .surface_format = from_vk(m_swapchain.get_surface_format().format),
            .depth_format = from_vk(m_device.get_supported_depth_format(false)),
        };
    }

    AcquiredImage
    VulkanBackend::acquire_next_image(VkSemaphore image_available) const {
        check(m_is_initialized);

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
            vk_verify(result);
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
            vk_verify(result);
        }

        return swapchain_result;
    }
} // namespace mantle
