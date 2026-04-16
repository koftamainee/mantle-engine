#include "../vulkan/vulkan_utils.h"
#include "types.h"

#include <fstream>
#include <ios>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "core/assert.h"
#include "renderer/types.h"

namespace mantle {

    ImageFormat from_vk(VkFormat format) {
        switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return ImageFormat::Rgba8;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return ImageFormat::Rgba8Srgb;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return ImageFormat::Bgra8Srgb;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return ImageFormat::Rgba16f;
        case VK_FORMAT_R32_SFLOAT:
            return ImageFormat::R32f;
        case VK_FORMAT_D16_UNORM:
            return ImageFormat::D16;
        case VK_FORMAT_D32_SFLOAT:
            return ImageFormat::D32;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return ImageFormat::D16S8;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return ImageFormat::D24S8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return ImageFormat::D32S8;
        case VK_FORMAT_UNDEFINED:
            return ImageFormat::Undefined;
        default:
            fatal(true, "unsupported VkFormat");
        }
    }

    VkFormat to_vk(ImageFormat format) {
        switch (format) {
        case ImageFormat::Rgba8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::Rgba8Srgb:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ImageFormat::Bgra8Srgb:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case ImageFormat::Rgba16f:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ImageFormat::R32f:
            return VK_FORMAT_R32_SFLOAT;
        case ImageFormat::D16:
            return VK_FORMAT_D16_UNORM;
        case ImageFormat::D32:
            return VK_FORMAT_D32_SFLOAT;
        case ImageFormat::D16S8:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case ImageFormat::D24S8:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case ImageFormat::D32S8:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case ImageFormat::Undefined:
            return VK_FORMAT_UNDEFINED;
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

        if (has(ImageUsage::Sampled))
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (has(ImageUsage::Storage))
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
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

    VkImageAspectFlags to_vk_aspect(ImageFormat format) {
        switch (format) {
        case ImageFormat::D16:
        case ImageFormat::D32:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case ImageFormat::D16S8:
        case ImageFormat::D24S8:
        case ImageFormat::D32S8:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case ImageFormat::Undefined:
            fatal(true, "Invalid image format");
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    VkBufferUsageFlags to_vk(BufferUsage usage) {
        using U = std::underlying_type_t<BufferUsage>;
        U u = static_cast<U>(usage);

        VkBufferUsageFlags flags = 0;

        auto has = [&](BufferUsage bit) {
            return (u & static_cast<U>(bit)) != 0;
        };

        if (has(BufferUsage::Vertex))
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (has(BufferUsage::Index))
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (has(BufferUsage::Uniform))
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (has(BufferUsage::Storage))
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (has(BufferUsage::Transfer))
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT;

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

    VkAttachmentLoadOp to_vk(AttachmentLoad load) {
        switch (load) {
        case AttachmentLoad::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoad::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoad::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            fatal(true, "unsupported AttachmentLoad");
        }
    }

    VkAttachmentStoreOp to_vk(AttachmentStore store) {
        switch (store) {
        case AttachmentStore::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case AttachmentStore::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:
            fatal(true, "unsupported AttachmentStore");
        }
    }
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
        case ImageLayout::ShaderReadOnly:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
        using U = std::underlying_type_t<PipelineStage>;
        U u = static_cast<U>(stage);

        VkPipelineStageFlags2 flags = 0;

        auto has = [&](PipelineStage bit) {
            return (u & static_cast<U>(bit)) != 0;
        };

        if (has(PipelineStage::None))
            flags |= VK_PIPELINE_STAGE_2_NONE;
        if (has(PipelineStage::Top))
            flags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        if (has(PipelineStage::Bottom))
            flags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        if (has(PipelineStage::AllCommands))
            flags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        if (has(PipelineStage::ComputeShader))
            flags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        if (has(PipelineStage::VertexShader))
            flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        if (has(PipelineStage::FragmentShader))
            flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (has(PipelineStage::ColorOutput))
            flags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (has(PipelineStage::EarlyDepth))
            flags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        if (has(PipelineStage::LateDepth))
            flags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        if (has(PipelineStage::Transfer))
            flags |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;

        return flags;
    }

    VkAccessFlags2 infer_image_access(ImageLayout layout, AccessType access) {
        VkAccessFlags2 read_flags = VK_ACCESS_2_NONE;
        VkAccessFlags2 write_flags = VK_ACCESS_2_NONE;

        switch (layout) {
        case ImageLayout::ColorAttachment:
            read_flags = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
            write_flags = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ImageLayout::DepthAttachment:
            read_flags = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            write_flags = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case ImageLayout::ShaderReadOnly:
            read_flags = VK_ACCESS_2_SHADER_READ_BIT;
            break;
        case ImageLayout::General:
            read_flags = VK_ACCESS_2_SHADER_READ_BIT;
            write_flags = VK_ACCESS_2_SHADER_WRITE_BIT;
            break;
        case ImageLayout::TransferSrc:
            read_flags = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        case ImageLayout::TransferDst:
            write_flags = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case ImageLayout::Present:
        case ImageLayout::Undefined:
            fatal(access != AccessType::None,
                  "ImageLayout::Present/Undefined must use AccessType::None");
            return VK_ACCESS_2_NONE;
        default:
            fatal(true, "unsupported ImageLayout");
        }

        const bool wants_read =
            (access == AccessType::Read || access == AccessType::ReadWrite);
        const bool wants_write =
            (access == AccessType::Write || access == AccessType::ReadWrite);

        fatal(wants_read && read_flags == VK_ACCESS_2_NONE,
              "ImageLayout does not support read access");
        fatal(wants_write && write_flags == VK_ACCESS_2_NONE,
              "ImageLayout does not support write access");

        switch (access) {
        case AccessType::None:
            return VK_ACCESS_2_NONE;
        case AccessType::Read:
            return read_flags;
        case AccessType::Write:
            return write_flags;
        case AccessType::ReadWrite:
            return read_flags | write_flags;
        default:
            fatal(true, "unsupported AccessType");
        }
    }

    VkSampleCountFlagBits to_vk(SampleCount count) {
        switch (count) {
        case SampleCount::x1:
            return VK_SAMPLE_COUNT_1_BIT;
        case SampleCount::x2:
            return VK_SAMPLE_COUNT_2_BIT;
        case SampleCount::x4:
            return VK_SAMPLE_COUNT_4_BIT;
        case SampleCount::x8:
            return VK_SAMPLE_COUNT_8_BIT;
        }
        fatal(true, "Unknown sample count");
    }

    VkShaderStageFlags to_vk(ShaderStage stage) {
        switch (stage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::TessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            fatal(true, "Unknown shader stage");
        }
    }

    VkFormat to_vk(VertexFormat format) {
        switch (format) {
        case VertexFormat::Float1:
            return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::Float2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::Float3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::Float4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:
            fatal(true, "Unknown vertex format");
        }
    }

    VkPrimitiveTopology to_vk(PrimitiveTopology topology) {
        switch (topology) {
        case PrimitiveTopology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:
            fatal(true, "Unknown primitive topology");
        }
    }

    VkPolygonMode to_vk(PolygonMode mode) {
        switch (mode) {
        case PolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        case PolygonMode::Point:
            return VK_POLYGON_MODE_POINT;
        default:
            fatal(true, "Unknown polygon mode");
        }
    }

    VkCullModeFlags to_vk(CullMode mode) {
        switch (mode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
        default:
            fatal(true, "Unknown cull mode");
        }
    }

    VkFrontFace to_vk(FrontFace face) {
        switch (face) {
        case FrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case FrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        default:
            fatal(true, "Unknown front face");
        }
    }

    VkCompareOp to_vk(CompareOp op) {
        switch (op) {
        case CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:
            return VK_COMPARE_OP_LESS;
        case CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            fatal(true, "Unknown compare op");
        }
    }

    VkStencilOp to_vk(StencilOp op) {
        switch (op) {
        case StencilOp::Keep:
            return VK_STENCIL_OP_KEEP;
        case StencilOp::Zero:
            return VK_STENCIL_OP_ZERO;
        case StencilOp::Replace:
            return VK_STENCIL_OP_REPLACE;
        case StencilOp::IncrementAndClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DecrementAndClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::Invert:
            return VK_STENCIL_OP_INVERT;
        case StencilOp::IncrementAndWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::DecrementAndWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:
            fatal(true, "Unknown stencil op");
        }
    }

    VkBlendFactor to_vk(BlendFactor factor) {
        switch (factor) {
        case BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::SrcAlphaSaturate:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendFactor::Src1Color:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendFactor::OneMinusSrc1Color:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendFactor::Src1Alpha:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendFactor::OneMinusSrc1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            fatal(true, "Unknown blend factor");
        }
    }

    VkBlendOp to_vk(BlendOp op) {
        switch (op) {
        case BlendOp::Add:
            return VK_BLEND_OP_ADD;
        case BlendOp::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOp::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::Min:
            return VK_BLEND_OP_MIN;
        case BlendOp::Max:
            return VK_BLEND_OP_MAX;
        default:
            fatal(true, "Unknown blend op");
        }
    }

    VkLogicOp to_vk(LogicOp op) {
        switch (op) {
        case LogicOp::Clear:
            return VK_LOGIC_OP_CLEAR;
        case LogicOp::And:
            return VK_LOGIC_OP_AND;
        case LogicOp::AndReverse:
            return VK_LOGIC_OP_AND_REVERSE;
        case LogicOp::Copy:
            return VK_LOGIC_OP_COPY;
        case LogicOp::AndInverted:
            return VK_LOGIC_OP_AND_INVERTED;
        case LogicOp::NoOp:
            return VK_LOGIC_OP_NO_OP;
        case LogicOp::Xor:
            return VK_LOGIC_OP_XOR;
        case LogicOp::Or:
            return VK_LOGIC_OP_OR;
        case LogicOp::Nor:
            return VK_LOGIC_OP_NOR;
        case LogicOp::Equivalent:
            return VK_LOGIC_OP_EQUIVALENT;
        case LogicOp::Invert:
            return VK_LOGIC_OP_INVERT;
        case LogicOp::OrReverse:
            return VK_LOGIC_OP_OR_REVERSE;
        case LogicOp::CopyInverted:
            return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOp::OrInverted:
            return VK_LOGIC_OP_OR_INVERTED;
        case LogicOp::Nand:
            return VK_LOGIC_OP_NAND;
        case LogicOp::Set:
            return VK_LOGIC_OP_SET;
        default:
            fatal(true, "Unknown logic op");
        }
    }

    VkColorComponentFlags to_vk_color_write_mask(u8 mask) {
        VkColorComponentFlags flags = 0;
        if (mask & 0x1)
            flags |= VK_COLOR_COMPONENT_R_BIT;
        if (mask & 0x2)
            flags |= VK_COLOR_COMPONENT_G_BIT;
        if (mask & 0x4)
            flags |= VK_COLOR_COMPONENT_B_BIT;
        if (mask & 0x8)
            flags |= VK_COLOR_COMPONENT_A_BIT;
        return flags;
    }

    VkAccessFlags2 infer_buffer_access(PipelineStage stage, AccessType access) {
        VkAccessFlags2 read_flags = VK_ACCESS_2_NONE;
        VkAccessFlags2 write_flags = VK_ACCESS_2_NONE;

        switch (stage) {
        case PipelineStage::Transfer:
            read_flags = VK_ACCESS_2_TRANSFER_READ_BIT;
            write_flags = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case PipelineStage::VertexShader:
        case PipelineStage::FragmentShader:
        case PipelineStage::ComputeShader:
            read_flags = VK_ACCESS_2_SHADER_READ_BIT;
            write_flags = VK_ACCESS_2_SHADER_WRITE_BIT;
            break;
        case PipelineStage::VertexInput:
            read_flags = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT |
                VK_ACCESS_2_INDEX_READ_BIT;
            break;
        case PipelineStage::DrawIndirect:
            read_flags = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
            break;
        case PipelineStage::Host:
            read_flags = VK_ACCESS_2_HOST_READ_BIT;
            write_flags = VK_ACCESS_2_HOST_WRITE_BIT;
            break;
        case PipelineStage::None:
            fatal(access != AccessType::None,
                  "PipelineStage::None must use AccessType::None");
            return VK_ACCESS_2_NONE;
        default:
            fatal(true, "unsupported PipelineStage for buffer access");
        }

        const bool wants_read =
            (access == AccessType::Read || access == AccessType::ReadWrite);
        const bool wants_write =
            (access == AccessType::Write || access == AccessType::ReadWrite);

        fatal(wants_read && read_flags == VK_ACCESS_2_NONE,
              "PipelineStage does not support read access for buffers");
        fatal(wants_write && write_flags == VK_ACCESS_2_NONE,
              "PipelineStage does not support write access for buffers");

        switch (access) {
        case AccessType::None:
            return VK_ACCESS_2_NONE;
        case AccessType::Read:
            return read_flags;
        case AccessType::Write:
            return write_flags;
        case AccessType::ReadWrite:
            return read_flags | write_flags;
        default:
            fatal(true, "unsupported AccessType");
        }
    }

    PipelineStage infer_stage(ImageLayout layout) {
        switch (layout) {
        case ImageLayout::Undefined:
            return PipelineStage::Top;
        case ImageLayout::ColorAttachment:
            return PipelineStage::ColorOutput;
        case ImageLayout::DepthAttachment:
            return PipelineStage::EarlyDepth | PipelineStage::LateDepth;
        case ImageLayout::ShaderReadOnly:
            return PipelineStage::FragmentShader;
        case ImageLayout::TransferSrc:
        case ImageLayout::TransferDst:
            return PipelineStage::Transfer;
        case ImageLayout::General:
            return PipelineStage::ComputeShader;
        case ImageLayout::Present:
            return PipelineStage::Bottom;
        default:
            return PipelineStage::AllCommands;
        }
    }

    AccessType infer_access(ImageLayout layout) {
        switch (layout) {
        case ImageLayout::ColorAttachment:
        case ImageLayout::DepthAttachment:
        case ImageLayout::TransferDst:
            return AccessType::Write;
        case ImageLayout::TransferSrc:
        case ImageLayout::ShaderReadOnly:
            return AccessType::Read;
        case ImageLayout::General:
            return AccessType::ReadWrite;
        case ImageLayout::Undefined:
        case ImageLayout::Present:
            return AccessType::None;
        default:
            fatal(true, "Unknown layout");
        }
    }
} // namespace mantle
