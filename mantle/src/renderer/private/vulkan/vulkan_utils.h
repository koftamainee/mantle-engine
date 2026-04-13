#pragma once
#include <string_view>
#include <vector>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "core/types.h"
#include "renderer/types.h"

namespace mantle {
    void load_spv(std::string_view path, std::pmr::vector<u32> &out);

    ImageFormat from_vk(VkFormat format);
    VkImageLayout to_vk(ImageLayout layout);
    VkPipelineStageFlags2 to_vk(PipelineStage stage);
    VkBufferUsageFlags to_vk(BufferUsage usage);
    VmaMemoryUsage to_vma(MemoryType type);
    VkAccessFlags2 infer_access(ImageLayout layout, bool is_src);
} // namespace mantle
