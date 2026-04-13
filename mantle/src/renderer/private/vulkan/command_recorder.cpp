#include "command_recorder.h"

#include <vulkan/vulkan_core.h>
#include "resources/gpu_resource_manager_internal.h"

#include "core/assert.h"

namespace mantle {
    namespace {
        VkImageLayout to_vk(ImageLayout layout) {
            switch (layout) {
            case ImageLayout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::General:
                return VK_IMAGE_LAYOUT_GENERAL;
            case ImageLayout::ColorAttachment:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthAttachment:
                return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            case ImageLayout::AttachmentOptimal:
                return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            case ImageLayout::ShaderReadOnly:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ImageLayout::ReadOnlyOptimal:
                return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
            case ImageLayout::TransferSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::TransferDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ImageLayout::Present:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            default:
                fatal(false, "unsupported ImageLayout");
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }
        }

        VkPipelineStageFlags2 to_vk(PipelineStage stage) {
            switch (stage) {
            case PipelineStage::None:
                return VK_PIPELINE_STAGE_2_NONE;
            case PipelineStage::Top:
                return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            case PipelineStage::Bottom:
                return VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            case PipelineStage::AllCommands:
                return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            case PipelineStage::AllGraphics:
                return VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
            case PipelineStage::VertexInput:
                return VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
            case PipelineStage::VertexShader:
                return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
            case PipelineStage::EarlyFragmentTests:
                return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
            case PipelineStage::FragmentShader:
                return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            case PipelineStage::LateFragmentTests:
                return VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
            case PipelineStage::ColorOutput:
                return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            case PipelineStage::ComputeShader:
                return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            case PipelineStage::Transfer:
                return VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
            case PipelineStage::Blit:
                return VK_PIPELINE_STAGE_2_BLIT_BIT;
            case PipelineStage::Copy:
                return VK_PIPELINE_STAGE_2_COPY_BIT;
            case PipelineStage::Resolve:
                return VK_PIPELINE_STAGE_2_RESOLVE_BIT;
            case PipelineStage::Clear:
                return VK_PIPELINE_STAGE_2_CLEAR_BIT;
            default:
                fatal(true, "unsupported PipelineStage");
            }
        }

        VkAccessFlags2 infer_access(ImageLayout layout, bool is_src) {
            switch (layout) {
            case ImageLayout::ColorAttachment:
                return is_src ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
                              : VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            case ImageLayout::DepthAttachment:
                return is_src ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                              : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            case ImageLayout::ShaderReadOnly:
                return VK_ACCESS_2_SHADER_READ_BIT;
            case ImageLayout::TransferSrc:
                return VK_ACCESS_2_TRANSFER_READ_BIT;
            case ImageLayout::TransferDst:
                return VK_ACCESS_2_TRANSFER_WRITE_BIT;
            case ImageLayout::Present:
            case ImageLayout::Undefined:
                return VK_ACCESS_2_NONE;
            default:
                return VK_ACCESS_2_MEMORY_READ_BIT |
                    VK_ACCESS_2_MEMORY_WRITE_BIT;
            }
        }
    } // namespace

    void CommandRecorder::set_command_buffer(VkCommandBuffer cmd) {
        m_cmd = cmd;
    }

    void CommandRecorder::set_resource_manager(GPUResourceManager *resources) {
        m_resources = resources;
    }

    void CommandRecorder::image_barrier(const ImageBarrier &barrier) const {
        VkImage image = m_resources->m_impl->get_vk_image(barrier.image);

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
                    .baseMipLevel = barrier.base_mip,
                    .levelCount = barrier.mip_count,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
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
        VkImageView color_view =
            m_resources->m_impl->get_vk_image_view(info.color.image);

        VkRenderingAttachmentInfo color_attachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = color_view,
            .imageLayout = to_vk(info.color.layout),
            .loadOp = info.color.load == AttachmentLoad::Clear
                ? VK_ATTACHMENT_LOAD_OP_CLEAR
                : info.color.load == AttachmentLoad::Load
                ? VK_ATTACHMENT_LOAD_OP_LOAD
                : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = info.color.store == AttachmentStore::Store
                ? VK_ATTACHMENT_STORE_OP_STORE
                : VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {.color = {.float32 = {info.color.clear_r,
                                                 info.color.clear_g,
                                                 info.color.clear_b,
                                                 info.color.clear_a}}},
        };

        VkRenderingAttachmentInfo depth_attachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView =
                m_resources->m_impl->get_vk_image_view(info.depth.image),
            .imageLayout = to_vk(info.depth.layout),
            .loadOp = info.depth.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                       : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.depthStencil = {.depth = info.depth.clear_value}},
        };

        VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {.offset = {0, 0},
                           .extent = {info.width, info.height}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment,
            .pDepthAttachment = &depth_attachment,
        };

        vkCmdBeginRendering(m_cmd, &rendering_info);
    }


    void
    CommandRecorder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) {
        auto *impl = m_resources->m_impl;
        m_current_layout = impl->get_vk_pipeline_layout(pipeline);
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          impl->get_vk_pipeline(pipeline));
    }

    void
    CommandRecorder::bind_compute_pipeline(ComputePipelineHandle pipeline) {
        auto *impl = m_resources->m_impl;
        m_current_layout = impl->get_vk_pipeline_layout(pipeline);
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          impl->get_vk_pipeline(pipeline));
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

    void CommandRecorder::dispatch(u32 x, u32 y, u32 z) const {
        vkCmdDispatch(m_cmd, x, y, z);
    }

    void CommandRecorder::copy_buffer(BufferHandle src, BufferHandle dst,
                                      usize size, usize src_offset,
                                      usize dst_offset) const {
        auto *impl = m_resources->m_impl;
        VkBufferCopy region = {
            .srcOffset = src_offset,
            .dstOffset = dst_offset,
            .size = size,
        };
        vkCmdCopyBuffer(m_cmd, impl->get_vk_buffer(src),
                        impl->get_vk_buffer(dst), 1, &region);
    }

    void CommandRecorder::copy_buffer_to_image(BufferHandle src,
                                               ImageHandle dst, u32 width,
                                               u32 height) const {
        auto *impl = m_resources->m_impl;
        VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1},
        };
        vkCmdCopyBufferToImage(
            m_cmd, impl->get_vk_buffer(src), impl->get_vk_image(dst),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void CommandRecorder::bind_descriptor_set() const {} // TODO
} // namespace mantle
