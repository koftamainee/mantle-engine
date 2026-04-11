#pragma once

#include "core/types.h"

#include "renderer/renderer.h"
#include "resources/vulkan_resource_manager.h"
#include "vulkan/vulkan_context.h"
#include "vulkan/vulkan_device.h"
#include "vulkan/vulkan_graphics_pipeline.h"
#include "vulkan/vulkan_swapchain.h"
#include "vulkan/vulkan_cpu_allocator.h"

#include "core/memory/pmr/persistent_resource.h"

namespace mantle {
    struct FrameData final {
        VkCommandBuffer cmd;
        VkFence in_flight;
    };

    struct Renderer::Impl final {
        VirtualHeap *heap = nullptr;
        ArenaAllocator *scratch_arena = nullptr;
        PersistentResource persistent_resource;

        TlsfAllocator tlsf_vulkan_allocator;
        VulkanCPUAllocator vulkan_cpu_allocator;

        VulkanContext graphics_context;
        VulkanDevice device;
        VulkanResourceManager resource_manager;
        VulkanSwapchain swapchain;
        VulkanGraphicsPipeline graphics_pipeline;

        std::pmr::vector<FrameData> frames;
        std::pmr::vector<VkSemaphore> acquire_semaphores;
        std::pmr::vector<VkSemaphore> render_semaphores;

        VulkanResourceManager::ResourceHandle depth_image;
        VkImageView depth_view;

        u32 current_frame = 0;
        u32 image_index = 0;
        u32 acquire_index = 0;
        bool swapchain_dirty = false;

        glm::mat4 view;
        glm::mat4 projection;

        static constexpr u8 frames_in_flight = 2;

        void init(const Window &window, VirtualHeap *in_heap,
                  ArenaAllocator *in_scratch_arena);
        void destroy();

        void create_frames();
        void destroy_frames();
        FrameData &get_current_frame();
        void create_depth_image(u32 width, u32 height);
        void destroy_depth_image();

      private:
        void create_frame(FrameData &frame) const;
        void destroy_frame(FrameData &frame) const;
    };
} // namespace mantle
