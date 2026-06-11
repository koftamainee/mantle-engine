// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "mantle/renderer/types.h"
#include "types.h"
#include "vulkan_gpu_allocator.h"

namespace mantle {
    struct BufferResource final {
        VkBuffer      buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        void         *mapped = nullptr;
        BufferDesc    desc = {};

        PipelineStage current_stage = PipelineStage::Top;
        AccessType    current_access = AccessType::None;

        u32 bindless_index = UINT32_MAX;
    };
    struct ImageResource final {
        VkImage       image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView   view = VK_NULL_HANDLE;
        ImageDesc     desc = {};

        ImageLayout current_layout = ImageLayout::Undefined;

        u32 bindless_sample_index = UINT32_MAX;
        u32 bindless_storage_index = UINT32_MAX;
    };
    struct SamplerResource final {
        VkSampler   sampler = VK_NULL_HANDLE;
        SamplerDesc desc = {};

        u32 bindless_index = UINT32_MAX;
    };
    struct ShaderResource final {
        VkShaderModule shader = VK_NULL_HANDLE;
    };
    struct GraphicsPipelineResource final {
        VkPipeline           pipeline = VK_NULL_HANDLE;
        VkPipelineLayout     layout = VK_NULL_HANDLE;
        GraphicsPipelineDesc desc = {};
    };
    struct ComputePipelineResource final {
        VkPipeline          pipeline = VK_NULL_HANDLE;
        VkPipelineLayout    layout = VK_NULL_HANDLE;
        ComputePipelineDesc desc = {};
    };
} // namespace mantle
