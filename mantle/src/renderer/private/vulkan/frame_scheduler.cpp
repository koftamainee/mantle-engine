// Copyright (c) 2026 Mantle. All rights reserved.

#include "frame_scheduler.h"

#include "core/assert.h"
#include "core/memory/persistent_allocator.h"
#include "resources/gpu_resource_manager_internal.h"
#include "vkassert.h"
#include "vulkan_backend.h"
#include "vulkan_utils.h"

namespace mantle {
    void FrameScheduler::init(VulkanBackend *backend, GPUResourceManager *resource_manager,
                              u32 frames_in_flight, VirtualHeap *heap) {
        MANTLE_CHECK(!m_is_initialized);
        MANTLE_CHECK(backend != nullptr);
        MANTLE_CHECK(frames_in_flight > 0);

        m_logger = spdlog::get("vulkan").get();
        m_frame_arena.init(heap->take(megabytes(1)));
        m_pmr = ArenaResource(&m_frame_arena);

        m_backend = backend;
        m_frames_in_flight = frames_in_flight;
        m_swapchain_image_count = m_backend->get_swapchain_info().image_count;

        PersistentAllocator alloc;
        alloc.init(m_backend->m_heap);
        m_frames = alloc.push<FrameData>(m_frames_in_flight);
        m_render_finished = alloc.push<VkSemaphore>(m_swapchain_image_count);

        m_command_pool = m_backend->m_device.create_command_pool(
            m_backend->m_device.get_queue_families().graphics_family);
        MANTLE_CHECK(m_command_pool != VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        VulkanDevice          &device = m_backend->m_device;
        VkDevice               vk_device = device.get_device();
        VkAllocationCallbacks *vk_callbacks = m_backend->m_vk_allocator.vk_allocator();


        for (usize i = 0; i < m_frames_in_flight; i++) {
            VkFence         fence = VK_NULL_HANDLE;
            VkSemaphore     image_available = VK_NULL_HANDLE;
            VkCommandBuffer cmd =
                device.create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_command_pool);

            MANTLE_VK_VERIFY(vkCreateFence(vk_device, &fence_info, vk_callbacks, &fence));
            MANTLE_VK_VERIFY(
                vkCreateSemaphore(vk_device, &semaphore_info, vk_callbacks, &image_available));

            m_frames[i] = {
                .fence = fence,
                .image_available = image_available,
                .cmd = cmd,
            };
        }

        for (usize i = 0; i < m_swapchain_image_count; i++) {
            VkSemaphore render_finished = nullptr;
            MANTLE_VK_VERIFY(
                vkCreateSemaphore(vk_device, &semaphore_info, vk_callbacks, &render_finished));
            m_render_finished[i] = render_finished;
        }

        m_recorder.set_resource_manager(resource_manager);
        m_recorder.set_arena(&m_pmr);

        m_is_initialized = true;
        m_logger->info("Frame scheduler initialized");
    }

    void FrameScheduler::destroy() {
        if (m_is_initialized) {
            VkDevice               vk_device = m_backend->m_device.get_device();
            VkAllocationCallbacks *vk_callbacks = m_backend->m_vk_allocator.vk_allocator();

            for (usize i = 0; i < m_frames_in_flight; i++) {
                vkDestroyFence(vk_device, m_frames[i].fence, vk_callbacks);
                vkDestroySemaphore(vk_device, m_frames[i].image_available, vk_callbacks);
            }

            for (usize i = 0; i < m_swapchain_image_count; i++) {
                vkDestroySemaphore(vk_device, m_render_finished[i], vk_callbacks);
            }

            vkDestroyCommandPool(vk_device, m_command_pool, vk_callbacks);

            m_is_initialized = false;
            m_logger->info("Frame scheduler destroyed");
        }
    }

    FrameResult FrameScheduler::begin_frame(FrameContext &out_ctx) {
        MANTLE_CHECK(m_is_initialized);
        FrameData    &frame = m_frames[m_current_frame];
        VulkanDevice &device = m_backend->m_device;
        VkDevice      vk_device = device.get_device();

        m_frame_arena.reset();

        MANTLE_VK_VERIFY(vkWaitForFences(vk_device, 1, &frame.fence, VK_TRUE, UINT64_MAX));

        AcquiredImage acquired =
            m_backend->acquire_next_image(m_frames[m_current_frame].image_available);

        if (acquired.result == SwapchainResult::OutOfDate) {
            return FrameResult::NeedsResize;
        }

        MANTLE_VK_VERIFY(vkResetFences(vk_device, 1, &frame.fence));

        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        MANTLE_VK_VERIFY(vkBeginCommandBuffer(frame.cmd, &begin_info));

        m_recorder.set_command_buffer(frame.cmd);

        out_ctx = {
            .cmd = &m_recorder,
            .image_index = acquired.image_index,
            .frame_index = m_current_frame,
        };

        return FrameResult::Ok;
    }

    FrameResult FrameScheduler::end_frame(const FrameContext &ctx) {
        MANTLE_CHECK(m_is_initialized);
        FrameData &frame = m_frames[ctx.frame_index];

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo         submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.image_available,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame.cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_render_finished[ctx.image_index],
        };

        MANTLE_VK_VERIFY(vkEndCommandBuffer(frame.cmd));

        MANTLE_VK_VERIFY(
            vkQueueSubmit(m_backend->m_device.get_graphics_queue(), 1, &submit_info, frame.fence));

        SwapchainResult result =
            m_backend->present(ctx.image_index, m_render_finished[ctx.image_index]);

        m_current_frame = (m_current_frame + 1) % m_frames_in_flight;

        if (result == SwapchainResult::OutOfDate || result == SwapchainResult::Suboptimal) {
            return FrameResult::NeedsResize;
        }

        return FrameResult::Ok;
    }

    void FrameScheduler::on_swapchain_rebuilt(u32 new_image_count) const {
        MANTLE_FATAL(m_frames_in_flight != std::min(m_frames_in_flight, new_image_count),
                     "Image count in swapchain changed. Should be unreachable");
    }
} // namespace mantle
