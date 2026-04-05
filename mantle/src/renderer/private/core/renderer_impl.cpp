#include "renderer_impl.h"

#include <spdlog/spdlog.h>
#include "../vulkan/vkassert.h"
#include "../vulkan/vulkan_utils.h"
#include "core/assert.h"
#include "window/window.h"

namespace mantle {
    void Renderer::Impl::create_frame(FrameData &frame) const {
        VkDevice vkdevice = device.get_device();

        VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vk_verify(vkCreateFence(vkdevice, &fence_info, nullptr, &frame.in_flight));

        frame.cmd = device.create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
    }

    void Renderer::Impl::destroy_frame(FrameData &frame) const {
        VkDevice vkdevice = device.get_device();
        if (frame.in_flight != VK_NULL_HANDLE) {
            vkDestroyFence(vkdevice, frame.in_flight, nullptr);
        }
        frame = {};
    }

    void Renderer::Impl::init(const Window &window) {
        graphics_context.init(window.get_native_window());
        VkInstance instance = graphics_context.get_instance();
        VkSurfaceKHR surface = graphics_context.get_surface();

        device.init(graphics_context.get_instance(), surface);
        VkDevice vkdevice = device.get_device();
        VkPhysicalDevice physical_device = device.get_physical_device();

        resource_manager.init(physical_device, vkdevice, instance);

        auto [width, height] = window.get_framebuffer_size();

        swapchain.init(vkdevice, surface,
                               device.get_swapchain_support_details(surface),
                               device.get_queue_families(), width, height);
        create_frames();

        VulkanGraphicsPipeline::Config pipeline_cfg = {
            .vert_entry = "vert_main",
            .frag_entry = "frag_main",
            .color_format = swapchain.get_surface_format().format,
        };

        std::vector<uint32_t> spv = load_spv("assets/shaders/triangle.spv");

        graphics_pipeline.init(vkdevice, pipeline_cfg, spv);

        gpu_resource_manager.init(resource_manager, device);
    }

    void Renderer::Impl::destroy() {
        vkDeviceWaitIdle(device.get_device());

        graphics_pipeline.destroy();
        destroy_frames();
        swapchain.destroy();
        resource_manager.destroy();
        device.destroy();
        graphics_context.destroy();
    }

    void Renderer::Impl::create_frames() {
        frames.resize(frames_in_flight);
        for (auto &frame : frames) {
            create_frame(frame);
        }

        uint32_t image_count = static_cast<uint32_t>(swapchain.get_images().size());

        acquire_semaphores.resize(image_count);
        render_semaphores.resize(image_count);

        VkSemaphoreCreateInfo sem_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        for (auto &sem : acquire_semaphores) {
            vk_verify(vkCreateSemaphore(device.get_device(), &sem_info, nullptr, &sem));
        }
        for (auto &sem : render_semaphores) {
            vk_verify(vkCreateSemaphore(device.get_device(), &sem_info, nullptr, &sem));
        }

        spdlog::info("FrameData object are created");
    }

    void Renderer::Impl::destroy_frames() {
        for (auto &frame : frames) {
            destroy_frame(frame);
        }
        frames.clear();

        for (auto &sem : acquire_semaphores) {
            if (sem != VK_NULL_HANDLE) {
                vkDestroySemaphore(device.get_device(), sem, nullptr);
            }
        }
        acquire_semaphores.clear();

        for (auto &sem : render_semaphores) {
            if (sem != VK_NULL_HANDLE) {
                vkDestroySemaphore(device.get_device(), sem, nullptr);
            }
        }
        render_semaphores.clear();

        spdlog::info("FrameData objects are destroyed");
    }

    FrameData &Renderer::Impl::get_current_frame() {
        return frames[current_frame];
    }
}