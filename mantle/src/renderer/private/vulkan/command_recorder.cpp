#include "command_recorder.h"

#include <vulkan/vulkan_core.h>
#include "resources/gpu_resource_manager_internal.h"
#include "vulkan/vulkan_utils.h"

#include "core/assert.h"
#include "spdlog/fmt/bundled/os.h"

namespace mantle {

    void CommandRecorder::set_command_buffer(VkCommandBuffer cmd) {
        m_cmd = cmd;
    }

    void CommandRecorder::set_resource_manager(GPUResourceManager *resources) {
        m_resources = resources;
    }

    void CommandRecorder::set_arena(ArenaResource *pmr) { m_pmr = pmr; }

    void CommandRecorder::image_barrier(const ImageBarrier &barrier) const {
        std::array<ImageBarrier, 1> arr{barrier};
        image_barriers(arr);
    }

    void CommandRecorder::image_barriers(
        std::span<const ImageBarrier> barriers) const {
        if (barriers.empty()) {
            return;
        }

        std::pmr::vector<VkImageMemoryBarrier2> vk_barriers(m_pmr);
        vk_barriers.reserve(barriers.size());

        for (const auto &barrier : barriers) {
            auto *image = barrier.image;
            check(image != nullptr);
            image->current_layout = barrier.to;

            vk_barriers.push_back({
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = to_vk(barrier.src_stage),
                .srcAccessMask = to_vk(barrier.src_access),
                .dstStageMask = to_vk(barrier.dst_stage),
                .dstAccessMask = to_vk(barrier.dst_access),
                .oldLayout = to_vk(barrier.from),
                .newLayout = to_vk(barrier.to),
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image->image,
                .subresourceRange =
                    {
                        .aspectMask = to_vk_aspect(image->desc.format),
                        .baseMipLevel = 0,
                        .levelCount = VK_REMAINING_MIP_LEVELS,
                        .baseArrayLayer = 0,
                        .layerCount = VK_REMAINING_ARRAY_LAYERS,
                    },
            });
        }

        VkDependencyInfo dep_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = static_cast<u32>(vk_barriers.size()),
            .pImageMemoryBarriers = vk_barriers.data(),
        };

        vkCmdPipelineBarrier2(m_cmd, &dep_info);
    }

    void CommandRecorder::buffer_barrier(const BufferBarrier &barrier) const {
        std::array<BufferBarrier, 1> arr{barrier};
        buffer_barriers(arr);
    }

    void CommandRecorder::buffer_barriers(
        std::span<const BufferBarrier> barriers) const {
        if (barriers.empty()) {
            return;
        }

        std::pmr::vector<VkBufferMemoryBarrier2> vk_barriers(m_pmr);
        vk_barriers.reserve(barriers.size());
        for (const auto &barrier : barriers) {
            auto &buffer = barrier.buffer;

            vk_barriers.push_back({
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcStageMask = to_vk(barrier.src_stage),
                .srcAccessMask = to_vk(barrier.src_access),
                .dstStageMask = to_vk(barrier.dst_stage),
                .dstAccessMask = to_vk(barrier.dst_access),
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = buffer->buffer,
                .offset = 0,
                .size = VK_WHOLE_SIZE,
            });
        }
        VkDependencyInfo dep_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = static_cast<u32>(vk_barriers.size()),
            .pBufferMemoryBarriers = vk_barriers.data(),
        };
        vkCmdPipelineBarrier2(m_cmd, &dep_info);
    }

    void CommandRecorder::begin_rendering(const RenderingInfo &info) const {
        std::pmr::vector<VkRenderingAttachmentInfo> color_attachments(m_pmr);
        color_attachments.reserve(info.colors.size());

        const VkRenderingAttachmentInfo *p_depth_attachment = nullptr;

        for (const auto &color : info.colors) {
            color_attachments.push_back({
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = color.image->view,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = to_vk(color.load),
                .storeOp = to_vk(color.store),
                .clearValue =
                    {.color = {.float32 = {color.clear_r, color.clear_g,
                                           color.clear_b, color.clear_a}}},
            });
        }

        VkRenderingAttachmentInfo depth_attachment;
        if (info.depth != nullptr) {
            check(info.depth->image != nullptr);
            depth_attachment = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = info.depth->image->view,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = to_vk(info.depth->load),
                .storeOp = to_vk(info.depth->store),
                .clearValue = {.depthStencil = {.depth =
                                                    info.depth->clear_value}},
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


    void CommandRecorder::bind_graphics_pipeline(
        const GraphicsPipelineResource &pipeline) {
        m_current_layout = pipeline.layout;
        m_push_constants = pipeline.desc.push_constants;
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline.pipeline);

        VkDescriptorSet bindless_set = m_resources->m_impl->get_bindless_set();
        vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_current_layout, 0, 1, &bindless_set, 0,
                                nullptr);
    }

    void
    CommandRecorder::bind_compute_pipeline(ComputePipelineResource &pipeline) {
        m_push_constants =
            std::span<PushConstantsRange>(&pipeline.desc.push_constants, 1);
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline.pipeline);

        VkDescriptorSet bindless_set = m_resources->m_impl->get_bindless_set();
        vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                m_current_layout, 0, 1, &bindless_set, 0,
                                nullptr);
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
        VkBufferCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
            .srcOffset = info.src_offset,
            .dstOffset = info.dst_offset,
            .size = info.size,
        };

        check(info.src != nullptr);
        check(info.dst != nullptr);

        VkCopyBufferInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
            .srcBuffer = info.src->buffer,
            .dstBuffer = info.dst->buffer,
            .regionCount = 1,
            .pRegions = &region,
        };

        vkCmdCopyBuffer2(m_cmd, &copy_info);
    }

    void CommandRecorder::copy_buffer_to_image(
        const BufferImageCopyInfo &info) const {
        check(info.src != nullptr);
        check(info.dst != nullptr);
        auto image = info.dst;


        u32 width = image->desc.width >> info.mip_level;
        u32 height = image->desc.height >> info.mip_level;

        VkBufferImageCopy2 region{
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .pNext = nullptr,
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

        VkCopyBufferToImageInfo2 copy_info{
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcBuffer = info.src->buffer,
            .dstImage = image->image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
        };

        vkCmdCopyBufferToImage2(m_cmd, &copy_info);
    }

    void CommandRecorder::copy_image(const ImageCopyInfo &info) const {
        auto *src = info.src;
        auto *dst = info.dst;

        check(src != nullptr);
        check(dst != nullptr);

        u32 src_width = src->desc.width >> info.src_mip_level;
        u32 src_height = src->desc.height >> info.src_mip_level;

        VkExtent3D extent = {
            .width = info.width ? info.width : src_width,
            .height = info.height ? info.height : src_height,
            .depth = 1,
        };

        VkImageCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
            .srcSubresource =
                {
                    .aspectMask = to_vk_aspect(src->desc.format),
                    .mipLevel = info.src_mip_level,
                    .baseArrayLayer = info.src_array_layer,
                    .layerCount = 1,
                },
            .srcOffset = {0, 0, 0},
            .dstSubresource =
                {
                    .aspectMask = to_vk_aspect(dst->desc.format),
                    .mipLevel = info.dst_mip_level,
                    .baseArrayLayer = info.dst_array_layer,
                    .layerCount = 1,
                },
            .dstOffset = {0, 0, 0},
            .extent = extent,
        };

        VkCopyImageInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
            .srcImage = src->image,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = dst->image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
        };

        vkCmdCopyImage2(m_cmd, &copy_info);
    }

    void CommandRecorder::blit_image(const ImageBlitInfo &info) const {
        auto *src = info.src;
        auto *dst = info.dst;

        check(src != nullptr);
        check(dst != nullptr);

        VkImageBlit2 region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .srcOffsets = {{0, 0, 0},
                           {static_cast<i32>(src->desc.width),
                            static_cast<i32>(src->desc.height), 1}},
            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .dstOffsets = {{0, 0, 0},
                           {static_cast<i32>(dst->desc.width),
                            static_cast<i32>(dst->desc.height), 1}},
        };
        VkBlitImageInfo2 blit_info = {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage = src->image,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = dst->image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
            .filter = VK_FILTER_LINEAR,
        };
        vkCmdBlitImage2(m_cmd, &blit_info);
    }

    void CommandRecorder::copy_image_to_buffer(
        const ImageBufferCopyInfo &info) const {
        auto &image = info.src;
        auto &buffer = info.dst;

        u32 width = image->desc.width >> info.mip_level;
        u32 height = image->desc.height >> info.mip_level;

        VkBufferImageCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = to_vk_aspect(image->desc.format),
                    .mipLevel = info.mip_level,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1},
        };

        VkCopyImageToBufferInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2,
            .srcImage = image->image,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstBuffer = buffer->buffer,
            .regionCount = 1,
            .pRegions = &region,
        };

        vkCmdCopyImageToBuffer2(m_cmd, &copy_info);
    }

    void CommandRecorder::clear_color_image(const ImageResource &image, f32 r,
                                            f32 g, f32 b, f32 a) const {
        VkClearColorValue clear = {.float32 = {r, g, b, a}};
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0,
                                         VK_REMAINING_MIP_LEVELS, 0,
                                         VK_REMAINING_ARRAY_LAYERS};
        vkCmdClearColorImage(m_cmd, image.image,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1,
                             &range);
    }

    void CommandRecorder::clear_depth_image(const ImageResource &image,
                                            f32 depth) const {
        VkClearDepthStencilValue clear = {
            .depth = depth,
            .stencil = 0,
        };

        VkImageSubresourceRange range = {
            .aspectMask = to_vk_aspect(image.desc.format),
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        };

        vkCmdClearDepthStencilImage(m_cmd, image.image,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    &clear, 1, &range);
    }

    void CommandRecorder::bind_vertex_buffer(const BufferResource &buffer,
                                             u32 binding, usize offset) const {
        vkCmdBindVertexBuffers(m_cmd, binding, 1, &buffer.buffer, &offset);
    }

    void CommandRecorder::bind_index_buffer(const BufferResource &buffer,
                                            usize offset) const {
        vkCmdBindIndexBuffer(m_cmd, buffer.buffer, offset,
                             VK_INDEX_TYPE_UINT32);
    }

    void CommandRecorder::push_constants(const void *data,
                                         ShaderStage stage) const {
        for (const auto &pc : m_push_constants) {
            if (pc.stage == stage) {
                vkCmdPushConstants(m_cmd, m_current_layout, to_vk(stage),
                                   pc.offset, pc.size, data);
                return;
            }
        }
        fatal(true, "Invalid stage for push constant");
    }
} // namespace mantle
