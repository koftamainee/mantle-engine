#pragma once

#include <vulkan/vulkan.h>


#include "renderer/gpu_resource_manager.h"

namespace mantle {
    struct GPUResourceManager::Impl final {
        VkImage get_vk_image(ImageHandle handle) const;
        VkImageView get_vk_image_view(ImageHandle handle) const;
        VkBuffer get_vk_buffer(BufferHandle handle) const;
        VkPipeline get_vk_pipeline(GraphicsPipelineHandle handle) const;
        VkPipeline get_vk_pipeline(ComputePipelineHandle handle) const;
        VkPipelineLayout get_vk_pipeline_layout(GraphicsPipelineHandle handle) const;
        VkPipelineLayout get_vk_pipeline_layout(ComputePipelineHandle handle) const;
    };
} // namespace mantle
