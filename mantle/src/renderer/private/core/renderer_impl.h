#pragma once
#include <vulkan/vulkan.h>

#include "../vulkan/vulkan_device.h"
#include "../vulkan/vulkan_context.h"
#include "../vulkan/vulkan_graphics_pipeline.h"
#include "../resources/vulkan_resource_manager.h"
#include "../vulkan/vulkan_swapchain.h"
#include "renderer/renderer.h"
#include "../resources/gpu_resource_manager_impl.h"

namespace mantle {
    struct FrameData final {
        VkCommandBuffer cmd;
        VkFence in_flight;
    };

    struct Renderer::Impl final {
        VulkanContext graphics_context;
        VulkanDevice device;
        VulkanResourceManager resource_manager;
        VulkanSwapchain swapchain;
        VulkanGraphicsPipeline graphics_pipeline;
        GPUResourceManager gpu_resource_manager;

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