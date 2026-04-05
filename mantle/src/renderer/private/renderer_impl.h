#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_allocator.h"
#include "vulkan_device.h"
#include "vulkan_graphics_context.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_swapchain.h"
#include "renderer/renderer.h"

namespace mantle {
    struct FrameData final {
        VkCommandBuffer cmd;
        VkFence in_flight;
    };

    struct Renderer::Impl final {
        VulkanGraphicsContext graphics_context;
        VulkanDevice device;
        VulkanAllocator allocator;
        VulkanSwapchain swapchain;
        VulkanGraphicsPipeline graphics_pipeline;

        std::vector<FrameData> frames;
        std::vector<VkSemaphore> acquire_semaphores;
        std::vector<VkSemaphore> render_semaphores;

        uint32_t current_frame = 0;
        uint32_t image_index = 0;
        uint32_t acquire_index = 0;
        bool swapchain_dirty = false;

        static constexpr uint8_t frames_in_flight = 2;

        void init(const Window &window);
        void destroy();

        void create_frames();
        void destroy_frames();
        FrameData &get_current_frame();

    private:
        void create_frame(FrameData &frame) const;
        void destroy_frame(FrameData &frame) const;
    };
}