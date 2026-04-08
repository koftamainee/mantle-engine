#pragma once

#include "core/types.h"

#include "renderer/renderer.h"
#include "resources/gpu_resource_manager_impl.h"
#include "resources/vulkan_resource_manager.h"
#include "vulkan/vulkan_context.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_graphics_pipeline.h"
#include "vulkan/vulkan_swapchain.h"

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

        VulkanResourceManager::ResourceHandle depth_image;
        VkImageView depth_view;

        u32 current_frame = 0;
        u32 image_index = 0;
        u32 acquire_index = 0;
        bool swapchain_dirty = false;

        glm::mat4 view;
        glm::mat4 projection;

        static constexpr u8 frames_in_flight = 2;

        void init(const Window &window);
        void destroy();

        void create_frames();
        void destroy_frames();
        FrameData &get_current_frame();

      private:
        void create_frame(FrameData &frame) const;
        void destroy_frame(FrameData &frame) const;
        void create_depth_image(u32 width, u32 height);
        void destroy_depth_image();
    };
} // namespace mantle
