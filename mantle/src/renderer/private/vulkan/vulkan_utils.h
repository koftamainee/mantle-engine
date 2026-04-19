#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "core/types.h"
#include "renderer/types.h"
#include "types.h"

namespace mantle {
    ImageFormat from_vk(VkFormat format);
    VkFormat to_vk(ImageFormat format);
    VkImageUsageFlags to_vk(ImageUsage usage);
    VkImageAspectFlags to_vk_aspect(ImageFormat format);
    VkBufferUsageFlags to_vk(BufferUsage usage);
    VmaMemoryUsage to_vma(MemoryType type);
    VkFilter to_vk(Filter filter);
    VkSamplerMipmapMode to_vk_mip(Filter filter);
    VkSamplerAddressMode to_vk(AddressMode mode);
    VkAttachmentLoadOp to_vk(AttachmentLoad load);
    VkAttachmentStoreOp to_vk(AttachmentStore store);
    VkShaderStageFlags to_vk(ShaderStage stage);
    VkFormat to_vk(VertexFormat format);
    VkPrimitiveTopology to_vk(PrimitiveTopology topology);
    VkPolygonMode to_vk(PolygonMode mode);
    VkCullModeFlags to_vk(CullMode mode);
    VkFrontFace to_vk(FrontFace face);
    VkCompareOp to_vk(CompareOp op);
    VkStencilOp to_vk(StencilOp op);
    VkBlendFactor to_vk(BlendFactor factor);
    VkBlendOp to_vk(BlendOp op);
    VkLogicOp to_vk(LogicOp op);
    VkImageLayout to_vk(ImageLayout layout);
    VkPipelineStageFlags2 to_vk(PipelineStage stage);
    VkSampleCountFlagBits to_vk(SampleCount count);
    VkColorComponentFlags to_vk(ColorWriteMask mask);
    VkAccessFlags2 to_vk(AccessType access);
} // namespace mantle
