#include <core/assert.h>
#include <renderer/renderer.h>
#include <spdlog/spdlog.h>
#include "../vulkan/vulkan_context.h"
#include "../vulkan/vulkan_device.h"
#include "../vulkan/vulkan_swapchain.h"

#include "renderer_impl.h"

#include "../vulkan/vkassert.h"

namespace mantle {

    Renderer::Renderer() = default;
    Renderer::~Renderer() { destroy(); }

    void Renderer::init(const Window &window) {
        check(!m_is_initialized);

        m_impl = std::make_unique<Impl>();
        m_impl->init(window);

        m_is_initialized = true;
        spdlog::info("Renderer Initialized");
    }

    void Renderer::destroy() {
        if (m_is_initialized) {

            m_impl->destroy();
            m_impl.reset();

            spdlog::info("Renderer Destroyed");
            m_is_initialized = false;
        }
    }

    void Renderer::set_camera(const glm::mat4 &view,
                              const glm::mat4 &projection) const {
        m_impl->view = view;
        m_impl->projection = projection;
    }

    Renderer::Result Renderer::begin_frame() const {
        check(m_is_initialized);

        if (m_impl->swapchain_dirty) {
            return Result::FrameNeedsResize;
        }

        FrameData &frame = m_impl->get_current_frame();
        VkDevice device = m_impl->device.get_device();

        vk_verify(
            vkWaitForFences(device, 1, &frame.in_flight, VK_TRUE, UINT64_MAX));
        vk_verify(vkResetFences(device, 1, &frame.in_flight));

        VkSemaphore acquire_sem =
            m_impl->acquire_semaphores[m_impl->acquire_index];

        VkResult result = vkAcquireNextImageKHR(
            device, m_impl->swapchain.get_swapchain(), UINT64_MAX, acquire_sem,
            VK_NULL_HANDLE, &m_impl->image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_impl->swapchain_dirty = true;
            return Result::FrameNeedsResize;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            vk_verify(result);
        }

        vk_verify(vkResetCommandBuffer(frame.cmd, 0));

        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        vk_verify(vkBeginCommandBuffer(frame.cmd, &begin_info));

        return Result::Ok;
    }

    Renderer::Result Renderer::end_frame() const {
        auto &frame = m_impl->get_current_frame();

        VkSemaphore acquire_sem =
            m_impl->acquire_semaphores[m_impl->acquire_index];
        VkSemaphore render_sem = m_impl->render_semaphores[m_impl->image_index];

        vk_verify(vkEndCommandBuffer(frame.cmd));

        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &acquire_sem,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame.cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &render_sem,
        };

        vk_verify(vkQueueSubmit(m_impl->device.get_graphics_queue(), 1, &submit,
                                frame.in_flight));

        VkSwapchainKHR swapchain = m_impl->swapchain.get_swapchain();

        VkPresentInfoKHR present = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_sem,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &m_impl->image_index,
        };

        VkResult result =
            vkQueuePresentKHR(m_impl->device.get_present_queue(), &present);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            m_impl->swapchain_dirty = true;
        }
        else {
            vk_verify(result);
        }

        m_impl->current_frame =
            (m_impl->current_frame + 1) % Impl::frames_in_flight;
        m_impl->acquire_index = (m_impl->acquire_index + 1) %
            static_cast<u32>(m_impl->acquire_semaphores.size());

        if (m_impl->swapchain_dirty) {
            return Result::FrameNeedsResize;
        }
        return Result::Ok;
    }

    void Renderer::begin_pass() const {
        check(m_is_initialized);
        auto &frame = m_impl->frames[m_impl->current_frame];

        VkImage image =
            m_impl->swapchain.get_images()[m_impl->image_index].image;

        VkImageMemoryBarrier barrier_to_attachment = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        vkCmdPipelineBarrier(frame.cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier_to_attachment);

        VkImageMemoryBarrier depth_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_impl->resource_manager.get_image(m_impl->depth_image),
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .levelCount = 1,
                    .layerCount = 1,
                },
        };

        vkCmdPipelineBarrier(frame.cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &depth_barrier);

        VkRenderingAttachmentInfo color_attachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView =
                m_impl->swapchain.get_images()[m_impl->image_index].view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };

        VkRenderingAttachmentInfo depth_attachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_impl->depth_view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {.depthStencil = {1.0f, 0}},
        };

        VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {.extent = m_impl->swapchain.get_extent()},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment,
            .pDepthAttachment = &depth_attachment,
        };

        vkCmdBeginRendering(frame.cmd, &rendering_info);

        auto extent = m_impl->swapchain.get_extent();

        m_impl->graphics_pipeline.bind(frame.cmd);

        VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = static_cast<f32>(extent.width),
            .height = static_cast<f32>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissors = {
            .offset = {0, 0},
            .extent = extent,
        };

        vkCmdSetViewport(frame.cmd, 0, 1, &viewport);
        vkCmdSetScissor(frame.cmd, 0, 1, &scissors);
    }

    void Renderer::end_pass() const {
        check(m_is_initialized);
        auto &frame = m_impl->get_current_frame();
        vkCmdEndRendering(frame.cmd);

        VkImage image =
            m_impl->swapchain.get_images()[m_impl->image_index].image;

        VkImageMemoryBarrier barrier_to_present = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        vkCmdPipelineBarrier(frame.cmd,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier_to_present);
    }

    Renderer::Result Renderer::draw_mesh(MeshHandle handle,
                                         const glm::mat4 &model) const {
        check(m_is_initialized);
        auto &frame = m_impl->get_current_frame();
        if (!m_impl->gpu_resource_manager.is_valid(handle)) {
            return Result::InvalidMeshHandle;
        }
        auto &mesh = m_impl->gpu_resource_manager.m_impl->get_mesh_data(handle);

        VkBuffer vb =
            m_impl->gpu_resource_manager.m_impl->vulkan_resources.get_buffer(
                mesh.vertex_buffer);
        VkBuffer ib =
            m_impl->gpu_resource_manager.m_impl->vulkan_resources.get_buffer(
                mesh.index_buffer);


        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(frame.cmd, 0, 1, &vb, &offset);
        vkCmdBindIndexBuffer(frame.cmd, ib, 0, VK_INDEX_TYPE_UINT32);

        glm::mat4 mvp = m_impl->projection * m_impl->view * model;
        vkCmdPushConstants(frame.cmd, m_impl->graphics_pipeline.get_layout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                           &mvp);

        vkCmdDrawIndexed(frame.cmd, mesh.index_count, 1, 0, 0, 0);

        return Result::Ok;
    }

    void Renderer::resize(u32 width, u32 height) const {
        check(m_is_initialized);
        VkDevice device = m_impl->device.get_device();
        VkSurfaceKHR surface = m_impl->graphics_context.get_surface();

        vkDeviceWaitIdle(device);

        u32 old_count = static_cast<u32>(m_impl->acquire_semaphores.size());

        m_impl->swapchain.destroy();
        m_impl->swapchain.init(
            device, surface,
            m_impl->device.get_swapchain_support_details(surface),
            m_impl->device.get_queue_families(), width, height);

        u32 new_count = static_cast<u32>(m_impl->swapchain.get_images().size());
        if (new_count != old_count) {
            for (auto &sem : m_impl->acquire_semaphores) {
                vkDestroySemaphore(device, sem, nullptr);
            }
            for (auto &sem : m_impl->render_semaphores) {
                vkDestroySemaphore(device, sem, nullptr);
            }

            m_impl->acquire_semaphores.resize(new_count);
            m_impl->render_semaphores.resize(new_count);

            VkSemaphoreCreateInfo sem_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            for (auto &sem : m_impl->acquire_semaphores) {
                vk_verify(vkCreateSemaphore(device, &sem_info, nullptr, &sem));
            }
            for (auto &sem : m_impl->render_semaphores) {
                vk_verify(vkCreateSemaphore(device, &sem_info, nullptr, &sem));
            }

            m_impl->acquire_index = 0;
        }

        m_impl->swapchain_dirty = false;
    }

    GPUResourceManager &Renderer::get_resource_manager() const {
        check(m_is_initialized);
        return m_impl->gpu_resource_manager;
    }
} // namespace mantle
