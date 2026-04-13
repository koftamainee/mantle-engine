#include "../vulkan/vulkan_utils.h"

#include <fstream>
#include <ios>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "core/assert.h"
#include "renderer/types.h"

namespace mantle {
    void load_spv(std::string_view path, std::pmr::vector<u32> &out) {
        std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
        check(file.is_open());

        long size = file.tellg();
        check(size % 4 == 0);

        file.seekg(0);
        out.resize(size / 4);
        file.read(reinterpret_cast<char *>(out.data()), size);
    }

    ImageFormat from_vk(VkFormat format) {
        switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return ImageFormat::Rgba8;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return ImageFormat::Rgba8Srgb;
        case VK_FORMAT_R16G16_UNORM:
            return ImageFormat::Rg16;
        case VK_FORMAT_R32_SFLOAT:
            return ImageFormat::R32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return ImageFormat::D32S8;
        case VK_FORMAT_D32_SFLOAT:
            return ImageFormat::D32;
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return ImageFormat::D24S8;
        case VK_FORMAT_D16_UNORM:
            return ImageFormat::D16;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return ImageFormat::Bgra8;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return ImageFormat::Bgra8Srgb;
        default:
            checkf(false, "unsupported VkFormat");
        }
    };

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

    VkBufferUsageFlags to_vk(BufferUsage usage) {
        VkBufferUsageFlags flags = 0;

        if (static_cast<int>(usage) & static_cast<int>(BufferUsage::Vertex)) {
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        if (static_cast<int>(usage) & static_cast<int>(BufferUsage::Index)) {
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }

        if (static_cast<int>(usage) & static_cast<int>(BufferUsage::Uniform)) {
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if (static_cast<int>(usage) & static_cast<int>(BufferUsage::Storage)) {
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        if (static_cast<int>(usage) & static_cast<int>(BufferUsage::Transfer)) {
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        return flags;
    }


    VmaMemoryUsage to_vma(MemoryType type) {
        switch (type) {
        case MemoryType::Gpu:
            return VMA_MEMORY_USAGE_GPU_ONLY;

        case MemoryType::CpuToGpu:
            return VMA_MEMORY_USAGE_CPU_TO_GPU;

        default:
            fatal(false, "unsupported MemoryType");
            return VMA_MEMORY_USAGE_UNKNOWN;
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
            return VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        }
    }

} // namespace mantle
