#include "command_recorder.h"

#include <vulkan/vulkan_core.h>
#include "resources/gpu_resource_manager_internal.h"
#include "vulkan/vulkan_utils.h"

#include "core/assert.h"

namespace mantle {

    void CommandRecorder::set_command_buffer(VkCommandBuffer cmd) {
        m_cmd = cmd;
    }

    void CommandRecorder::set_resource_manager(GPUResourceManager *resources) {
        m_resources = resources;
    }

    void CommandRecorder::image_barrier(const ImageBarrier &barrier) const {
        VkImage image = m_resources->m_impl->get_image(barrier.image).image;

        VkImageMemoryBarrier2 vk_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = to_vk(barrier.src_stage),
            .srcAccessMask = infer_access(barrier.from, true),
            .dstStageMask = to_vk(barrier.dst_stage),
            .dstAccessMask = infer_access(barrier.to, false),
            .oldLayout = to_vk(barrier.from),
            .newLayout = to_vk(barrier.to),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                },
        };

        VkDependencyInfo dep_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &vk_barrier,
        };

        vkCmdPipelineBarrier2(m_cmd, &dep_info);
    }

    void CommandRecorder::pipeline_barriers(
        std::span<const ImageBarrier> barriers) const {
        for (const auto &barrier : barriers) {
            image_barrier(barrier);
        }
    }

    void CommandRecorder::begin_rendering(const RenderingInfo &info) const {
        std::vector<VkRenderingAttachmentInfo>
            color_attachments; // TODO: use custom scratch arena alloc
        color_attachments.reserve(info.colors.size());

        const VkRenderingAttachmentInfo *p_depth_attachment = nullptr;

        for (const auto &color : info.colors) {
            if (color.image.index != 0) {
                color_attachments.push_back({
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView =
                        m_resources->m_impl->get_image(color.image).view,
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = to_vk(color.load),
                    .storeOp = to_vk(color.store),
                    .clearValue =
                        {.color = {.float32 = {color.clear_r, color.clear_g,
                                               color.clear_b, color.clear_a}}},
                });
            }
        }

        VkRenderingAttachmentInfo depth_attachment;
        if (info.depth.image.index != 0) {
            depth_attachment = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView =
                    m_resources->m_impl->get_image(info.depth.image).view,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = to_vk(info.depth.load),
                .storeOp = to_vk(info.depth.store),
                .clearValue = {.depthStencil = {.depth =
                                                    info.depth.clear_value}},
            };
            p_depth_attachment = &depth_attachment;
        }

        VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {.offset = {0, 0},
                           .extent = {info.width, info.height}},
            .layerCount = 1,
            .colorAttachmentCount =
                static_cast<uint32_t>(color_attachments.size()),
            .pColorAttachments = color_attachments.data(),
            .pDepthAttachment = p_depth_attachment,
        };

        vkCmdBeginRendering(m_cmd, &rendering_info);
    }

    void CommandRecorder::end_rendering() const { vkCmdEndRendering(m_cmd); }


    void
    CommandRecorder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) {
        auto *impl = m_resources->m_impl;
        auto &pipeline_ref = impl->get_graphics_pipeline(pipeline);
        m_current_layout = pipeline_ref.layout;
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline_ref.pipeline);
    }

    void
    CommandRecorder::bind_compute_pipeline(ComputePipelineHandle pipeline) {
        auto *impl = m_resources->m_impl;
        auto &pipeline_ref = impl->get_compute_pipeline(pipeline);
        m_current_layout = pipeline_ref.layout;
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline_ref.pipeline);
    }

    void CommandRecorder::push_constants(const void *data, u32 size) const {
        vkCmdPushConstants(m_cmd, m_current_layout, VK_SHADER_STAGE_ALL, 0,
                           size, data);
    }

    void CommandRecorder::set_viewport(f32 x, f32 y, f32 width,
                                       f32 height) const {
        VkViewport viewport = {
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(m_cmd, 0, 1, &viewport);
    }

    void CommandRecorder::set_scissor(i32 x, i32 y, u32 width,
                                      u32 height) const {
        VkRect2D scissor = {
            .offset = {.x = x, .y = y},
            .extent = {.width = width, .height = height},
        };
        vkCmdSetScissor(m_cmd, 0, 1, &scissor);
    }

    void CommandRecorder::draw(const DrawInfo &info) const {
        vkCmdDraw(m_cmd, info.vertex_count, info.instance_count,
                  info.first_vertex, info.first_instance);
    }

    void CommandRecorder::draw_indexed(const DrawIndexedInfo &info) const {
        vkCmdDrawIndexed(m_cmd, info.index_count, info.instance_count,
                         info.first_index, info.vertex_offset,
                         info.first_instance);
    }

    void CommandRecorder::dispatch(const DispatchInfo &info) const {
        vkCmdDispatch(m_cmd, info.x, info.y, info.z);
    }

    void CommandRecorder::copy_buffer(const BufferCopyInfo &info) const {
        auto *impl = m_resources->m_impl;
        VkBufferCopy region = {
            .srcOffset = info.src_offset,
            .dstOffset = info.dst_offset,
            .size = info.size,
        };
        vkCmdCopyBuffer(m_cmd, impl->get_buffer(info.src).buffer,
                        impl->get_buffer(info.dst).buffer, 1, &region);
    }

    void CommandRecorder::copy_buffer_to_image(
        const BufferImageCopyInfo &info) const {
        auto *impl = m_resources->m_impl;
        auto &image = impl->get_image(info.dst);
        u32 width = image.desc.width >> info.mip_level;
        u32 height = image.desc.height >> info.mip_level;

        VkBufferImageCopy region = {
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = info.mip_level,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1},
        };

        vkCmdCopyBufferToImage(
            m_cmd, impl->get_buffer(info.src).buffer, image.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }
    void CommandRecorder::bind_descriptor_set() const {} // TODO
} // namespace mantle
