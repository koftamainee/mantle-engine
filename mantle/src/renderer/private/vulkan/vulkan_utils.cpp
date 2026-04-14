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
        fatal(!file.is_open(), "failed to open shader file");

        long size = file.tellg();
        fatal(size % 4 != 0, "Invalid SPIR-V shader");

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
            fatal(true, "unsupported VkFormat");
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
            fatal(true, "unsupported ImageLayout");
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
            fatal(true, "unsupported MemoryType");
        }
    }

    VkFormat to_vk(ImageFormat format) {
        switch (format) {
        case ImageFormat::Rgba8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::Rgba8Srgb:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ImageFormat::Bgra8:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case ImageFormat::Bgra8Srgb:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case ImageFormat::Rgba16:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case ImageFormat::Rgba32:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ImageFormat::Rg16:
            return VK_FORMAT_R16G16_UNORM;
        case ImageFormat::R32:
            return VK_FORMAT_R32_SFLOAT;
        case ImageFormat::D16:
            return VK_FORMAT_D16_UNORM;
        case ImageFormat::D24S8:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case ImageFormat::D32:
            return VK_FORMAT_D32_SFLOAT;
        case ImageFormat::D32S8:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:
            fatal(true, "unsupported ImageFormat");
        }
    }

    VkImageUsageFlags to_vk(ImageUsage usage) {
        using U = std::underlying_type_t<ImageUsage>;
        U u = static_cast<U>(usage);

        VkImageUsageFlags flags = 0;

        auto has = [&](ImageUsage bit) {
            return (u & static_cast<U>(bit)) != 0;
        };

        if (has(ImageUsage::Storage))
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;

        if (has(ImageUsage::Sampled))
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

        if (has(ImageUsage::Color))
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (has(ImageUsage::Depth))
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (has(ImageUsage::TransferSrc))
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if (has(ImageUsage::TransferDst))
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        return flags;
    }

    VkSampleCountFlagBits to_vk(u32 sample_count) {
        switch (sample_count) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        case 64:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            fatal(true, "unsupported sample count");
        }
    }

    VkImageAspectFlags to_vk_aspect(ImageFormat format) {
        switch (format) {
        case ImageFormat::D16:
        case ImageFormat::D32:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case ImageFormat::D24S8:
        case ImageFormat::D32S8:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    VkFilter to_vk(Filter filter) {
        switch (filter) {
        case Filter::Nearest:
            return VK_FILTER_NEAREST;
        case Filter::Linear:
            return VK_FILTER_LINEAR;
        default:
            fatal(true, "unsupported Filter");
        }
    }

    VkSamplerMipmapMode to_vk_mip(Filter filter) {
        switch (filter) {
        case Filter::Nearest:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case Filter::Linear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            fatal(true, "unsupported Filter");
        }
    }

    VkSamplerAddressMode to_vk(AddressMode mode) {
        switch (mode) {
        case AddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            fatal(true, "unsupported AddressMode");
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
        case ImageLayout::ReadOnlyOptimal:
            return VK_ACCESS_2_SHADER_READ_BIT |
                   VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        case ImageLayout::AttachmentOptimal:
            return is_src ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                          : VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case ImageLayout::TransferSrc:
            return VK_ACCESS_2_TRANSFER_READ_BIT;
        case ImageLayout::TransferDst:
            return VK_ACCESS_2_TRANSFER_WRITE_BIT;
        case ImageLayout::General:
            return VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        case ImageLayout::Present:
        case ImageLayout::Undefined:
            return VK_ACCESS_2_NONE;
        default:
            fatal(true, "unsupported ImageLayout");
        }
    }

} // namespace mantle