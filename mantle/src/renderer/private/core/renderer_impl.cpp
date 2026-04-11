#include "renderer_impl.h"

#include <spdlog/spdlog.h>
#include "core/assert.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"
#include "vulkan/vkassert.h"
#include "vulkan/vulkan_utils.h"
#include "window/window.h"

namespace mantle {
    void Renderer::Impl::init(const Window &window, VirtualHeap *in_heap,
                              ArenaAllocator *in_scratch_arena) {
        heap = in_heap;
        scratch_arena = in_scratch_arena;
        graphics_context.init(window.get_native_window(), scratch_arena);
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


        persistent_resource = PersistentResource(heap);

        frames = std::pmr::vector<FrameData>(&persistent_resource);
        acquire_semaphores =
            std::pmr::vector<VkSemaphore>(&persistent_resource);
        render_semaphores = std::pmr::vector<VkSemaphore>(&persistent_resource);

        create_frames();

        VulkanGraphicsPipeline::Config pipeline_cfg = {
            .vert_entry = "vert_main",
            .frag_entry = "frag_main",
            .color_format = swapchain.get_surface_format().format,
        };

        ScopeArena scope_arena(scratch_arena);

        ArenaResource resource(scratch_arena);
        std::pmr::vector<u32> spv(&resource);

        load_spv("assets/shaders/flat.spv", spv);

        graphics_pipeline.init(vkdevice, pipeline_cfg, spv);

        gpu_resource_manager.init(resource_manager, device);

        create_depth_image(width, height);
    }

    void Renderer::Impl::destroy() {
        vkDeviceWaitIdle(device.get_device());

        destroy_depth_image();
        graphics_pipeline.destroy();
        destroy_frames();
        swapchain.destroy();
        resource_manager.destroy();
        device.destroy();
        graphics_context.destroy();
    }

    void Renderer::Impl::create_frame(FrameData &frame) const {
        VkDevice vkdevice = device.get_device();

        VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vk_verify(
            vkCreateFence(vkdevice, &fence_info, nullptr, &frame.in_flight));

        frame.cmd = device.create_command_buffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
    }

    void Renderer::Impl::destroy_frame(FrameData &frame) const {
        VkDevice vkdevice = device.get_device();
        if (frame.in_flight != VK_NULL_HANDLE) {
            vkDestroyFence(vkdevice, frame.in_flight, nullptr);
        }
        frame = {};
    }

    void Renderer::Impl::create_depth_image(u32 width, u32 height) {

        VkImageCreateInfo create_image_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_D32_SFLOAT,
            .extent = {width, height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        depth_image = resource_manager.create_image(create_image_info,
                                                    VMA_MEMORY_USAGE_GPU_ONLY);

        VkImageViewCreateInfo view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = resource_manager.get_image(depth_image),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_D32_SFLOAT,
            .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
        };

        vk_verify(vkCreateImageView(device.get_device(), &view_create_info,
                                    nullptr, &depth_view));
    }
    void Renderer::Impl::destroy_depth_image() {
        vkDestroyImageView(device.get_device(), depth_view, nullptr);
        resource_manager.destroy_image(depth_image, true);
        depth_view = VK_NULL_HANDLE;
    }

    void Renderer::Impl::create_frames() {
        frames.resize(frames_in_flight);
        for (auto &frame : frames) {
            create_frame(frame);
        }

        auto image_count = static_cast<u32>(swapchain.get_images().size());

        acquire_semaphores.resize(image_count);
        render_semaphores.resize(image_count);

        VkSemaphoreCreateInfo sem_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        for (auto &sem : acquire_semaphores) {
            vk_verify(vkCreateSemaphore(device.get_device(), &sem_info, nullptr,
                                        &sem));
        }
        for (auto &sem : render_semaphores) {
            vk_verify(vkCreateSemaphore(device.get_device(), &sem_info, nullptr,
                                        &sem));
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
} // namespace mantle
