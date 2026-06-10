// Copyright (c) 2026 Mantle. All rights reserved.

#include "../vulkan/vulkan_utils.h"

#include <fstream>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "core/assert.h"
#include "renderer/types.h"
#include "types.h"

namespace mantle {

    ImageFormat from_vk(VkFormat format) {
        ImageFormat result = ImageFormat::Undefined;

        switch (format) {
            case VK_FORMAT_R8G8B8A8_UNORM: {
                result = ImageFormat::Rgba8;
            } break;
            case VK_FORMAT_R8G8B8A8_SRGB: {
                result = ImageFormat::Rgba8Srgb;
            } break;
            case VK_FORMAT_B8G8R8A8_SRGB: {
                result = ImageFormat::Bgra8Srgb;
            } break;
            case VK_FORMAT_R16G16B16A16_SFLOAT: {
                result = ImageFormat::Rgba16f;
            } break;
            case VK_FORMAT_R32_SFLOAT: {
                result = ImageFormat::R32f;
            } break;
            case VK_FORMAT_D16_UNORM: {
                result = ImageFormat::D16;
            } break;
            case VK_FORMAT_D32_SFLOAT: {
                result = ImageFormat::D32;
            } break;
            case VK_FORMAT_D16_UNORM_S8_UINT: {
                result = ImageFormat::D16S8;
            } break;
            case VK_FORMAT_D24_UNORM_S8_UINT: {
                result = ImageFormat::D24S8;
            } break;
            case VK_FORMAT_D32_SFLOAT_S8_UINT: {
                result = ImageFormat::D32S8;
            } break;
            case VK_FORMAT_UNDEFINED: {
                result = ImageFormat::Undefined;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported VkFormat");
            } break;
        }

        return result;
    }

    VkFormat to_vk(ImageFormat format) {
        VkFormat result = VK_FORMAT_UNDEFINED;

        switch (format) {
            case ImageFormat::Rgba8: {
                result = VK_FORMAT_R8G8B8A8_UNORM;
            } break;
            case ImageFormat::Rgba8Srgb: {
                result = VK_FORMAT_R8G8B8A8_SRGB;
            } break;
            case ImageFormat::Bgra8Srgb: {
                result = VK_FORMAT_B8G8R8A8_SRGB;
            } break;
            case ImageFormat::Rgba16f: {
                result = VK_FORMAT_R16G16B16A16_SFLOAT;
            } break;
            case ImageFormat::R32f: {
                result = VK_FORMAT_R32_SFLOAT;
            } break;
            case ImageFormat::D16: {
                result = VK_FORMAT_D16_UNORM;
            } break;
            case ImageFormat::D32: {
                result = VK_FORMAT_D32_SFLOAT;
            } break;
            case ImageFormat::D16S8: {
                result = VK_FORMAT_D16_UNORM_S8_UINT;
            } break;
            case ImageFormat::D24S8: {
                result = VK_FORMAT_D24_UNORM_S8_UINT;
            } break;
            case ImageFormat::D32S8: {
                result = VK_FORMAT_D32_SFLOAT_S8_UINT;
            } break;
            case ImageFormat::Undefined: {
                result = VK_FORMAT_UNDEFINED;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported ImageFormat");
            } break;
        }

        return result;
    }

    VkImageUsageFlags to_vk(ImageUsage usage) {
        using U = std::underlying_type_t<ImageUsage>;
        U u = static_cast<U>(usage);

        VkImageUsageFlags flags = 0;

        auto has = [&](ImageUsage bit) { return (u & static_cast<U>(bit)) != 0; };

        if (has(ImageUsage::Sampled)) {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (has(ImageUsage::Storage)) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if (has(ImageUsage::Color)) {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (has(ImageUsage::Depth)) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (has(ImageUsage::TransferSrc)) {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if (has(ImageUsage::TransferDst)) {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        return flags;
    }

    VkImageAspectFlags to_vk_aspect(ImageFormat format) {
        VkImageAspectFlags result = VK_IMAGE_ASPECT_COLOR_BIT;

        switch (format) {
            case ImageFormat::D16:
            case ImageFormat::D32: {
                result = VK_IMAGE_ASPECT_DEPTH_BIT;
            } break;
            case ImageFormat::D16S8:
            case ImageFormat::D24S8:
            case ImageFormat::D32S8: {
                result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            } break;
            case ImageFormat::Undefined: {
                MANTLE_FATAL(true, "Invalid image format");
            } break;
            default: {
                result = VK_IMAGE_ASPECT_COLOR_BIT;
            } break;
        }

        return result;
    }

    VkBufferUsageFlags to_vk(BufferUsage usage) {
        using U = std::underlying_type_t<BufferUsage>;
        U u = static_cast<U>(usage);

        VkBufferUsageFlags flags = 0;

        auto has = [&](BufferUsage bit) { return (u & static_cast<U>(bit)) != 0; };

        if (has(BufferUsage::Vertex)) {
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (has(BufferUsage::Index)) {
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (has(BufferUsage::Uniform)) {
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if (has(BufferUsage::Storage)) {
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        if (has(BufferUsage::Indirect)) {
            flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }
        if (has(BufferUsage::Transfer)) {
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        if (has(BufferUsage::Indirect)) {
            flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }

        return flags;
    }

    VmaMemoryUsage to_vma(MemoryType type) {
        VmaMemoryUsage result = VMA_MEMORY_USAGE_GPU_ONLY;

        switch (type) {
            case MemoryType::Gpu: {
                result = VMA_MEMORY_USAGE_GPU_ONLY;
            } break;
            case MemoryType::CpuToGpu: {
                result = VMA_MEMORY_USAGE_CPU_TO_GPU;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported MemoryType");
            } break;
        }

        return result;
    }

    VkFilter to_vk(Filter filter) {
        VkFilter result = VK_FILTER_NEAREST;

        switch (filter) {
            case Filter::Nearest: {
                result = VK_FILTER_NEAREST;
            } break;
            case Filter::Linear: {
                result = VK_FILTER_LINEAR;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported Filter");
            } break;
        }

        return result;
    }

    VkSamplerMipmapMode to_vk_mip(Filter filter) {
        VkSamplerMipmapMode result = VK_SAMPLER_MIPMAP_MODE_NEAREST;

        switch (filter) {
            case Filter::Nearest: {
                result = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            } break;
            case Filter::Linear: {
                result = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported Filter");
            } break;
        }

        return result;
    }

    VkSamplerAddressMode to_vk(AddressMode mode) {
        VkSamplerAddressMode result = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        switch (mode) {
            case AddressMode::Repeat: {
                result = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            } break;
            case AddressMode::ClampToEdge: {
                result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            } break;
            case AddressMode::ClampToBorder: {
                result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported AddressMode");
            } break;
        }

        return result;
    }

    VkAttachmentLoadOp to_vk(AttachmentLoad load) {
        VkAttachmentLoadOp result = VK_ATTACHMENT_LOAD_OP_CLEAR;

        switch (load) {
            case AttachmentLoad::Clear: {
                result = VK_ATTACHMENT_LOAD_OP_CLEAR;
            } break;
            case AttachmentLoad::Load: {
                result = VK_ATTACHMENT_LOAD_OP_LOAD;
            } break;
            case AttachmentLoad::DontCare: {
                result = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported AttachmentLoad");
            } break;
        }

        return result;
    }

    VkAttachmentStoreOp to_vk(AttachmentStore store) {
        VkAttachmentStoreOp result = VK_ATTACHMENT_STORE_OP_STORE;

        switch (store) {
            case AttachmentStore::Store: {
                result = VK_ATTACHMENT_STORE_OP_STORE;
            } break;
            case AttachmentStore::DontCare: {
                result = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported AttachmentStore");
            } break;
        }

        return result;
    }

    VkImageLayout to_vk(ImageLayout layout) {
        VkImageLayout result = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (layout) {
            case ImageLayout::Undefined: {
                result = VK_IMAGE_LAYOUT_UNDEFINED;
            } break;
            case ImageLayout::General: {
                result = VK_IMAGE_LAYOUT_GENERAL;
            } break;
            case ImageLayout::ColorAttachment: {
                result = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            } break;
            case ImageLayout::DepthAttachment: {
                result = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            } break;
            case ImageLayout::ShaderReadOnly: {
                result = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            } break;
            case ImageLayout::TransferSrc: {
                result = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            } break;
            case ImageLayout::TransferDst: {
                result = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            } break;
            case ImageLayout::Present: {
                result = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported ImageLayout");
            } break;
        }

        return result;
    }

    VkPipelineStageFlags2 to_vk(PipelineStage stage) {
        using U = std::underlying_type_t<PipelineStage>;
        U u = static_cast<U>(stage);

        VkPipelineStageFlags2 flags = 0;

        auto has = [&](PipelineStage bit) { return (u & static_cast<U>(bit)) != 0; };

        if (has(PipelineStage::None)) {
            flags |= VK_PIPELINE_STAGE_2_NONE;
        }
        if (has(PipelineStage::Top)) {
            flags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        }
        if (has(PipelineStage::Bottom)) {
            flags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        }
        if (has(PipelineStage::AllCommands)) {
            flags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        }
        if (has(PipelineStage::ComputeShader)) {
            flags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        }
        if (has(PipelineStage::VertexShader)) {
            flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        }
        if (has(PipelineStage::FragmentShader)) {
            flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        }
        if (has(PipelineStage::ColorOutput)) {
            flags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        if (has(PipelineStage::EarlyDepth)) {
            flags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        }
        if (has(PipelineStage::LateDepth)) {
            flags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        }
        if (has(PipelineStage::Transfer)) {
            flags |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        }
        if (has(PipelineStage::VertexInput)) {
            flags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
        }
        if (has(PipelineStage::DrawIndirect)) {
            flags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        }
        if (has(PipelineStage::Host)) {
            flags |= VK_PIPELINE_STAGE_2_HOST_BIT;
        }

        return flags;
    }

    VkSampleCountFlagBits to_vk(SampleCount count) {
        VkSampleCountFlagBits result = VK_SAMPLE_COUNT_1_BIT;

        switch (count) {
            case SampleCount::x1: {
                result = VK_SAMPLE_COUNT_1_BIT;
            } break;
            case SampleCount::x2: {
                result = VK_SAMPLE_COUNT_2_BIT;
            } break;
            case SampleCount::x4: {
                result = VK_SAMPLE_COUNT_4_BIT;
            } break;
            case SampleCount::x8: {
                result = VK_SAMPLE_COUNT_8_BIT;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown sample count");
            } break;
        }

        return result;
    }

    VkShaderStageFlags to_vk(ShaderStage stage) {
        VkShaderStageFlags result = VK_SHADER_STAGE_VERTEX_BIT;

        switch (stage) {
            case ShaderStage::Vertex: {
                result = VK_SHADER_STAGE_VERTEX_BIT;
            } break;
            case ShaderStage::TessellationControl: {
                result = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            } break;
            case ShaderStage::TessellationEvaluation: {
                result = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            } break;
            case ShaderStage::Fragment: {
                result = VK_SHADER_STAGE_FRAGMENT_BIT;
            } break;
            case ShaderStage::Compute: {
                result = VK_SHADER_STAGE_COMPUTE_BIT;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown shader stage");
            } break;
        }

        return result;
    }

    VkFormat to_vk(VertexFormat format) {
        VkFormat result = VK_FORMAT_R32_SFLOAT;

        switch (format) {
            case VertexFormat::Float1: {
                result = VK_FORMAT_R32_SFLOAT;
            } break;
            case VertexFormat::Float2: {
                result = VK_FORMAT_R32G32_SFLOAT;
            } break;
            case VertexFormat::Float3: {
                result = VK_FORMAT_R32G32B32_SFLOAT;
            } break;
            case VertexFormat::Float4: {
                result = VK_FORMAT_R32G32B32A32_SFLOAT;
            } break;
            case VertexFormat::Uint1: {
                result = VK_FORMAT_R32_UINT;
            } break;
            case VertexFormat::Uint2: {
                result = VK_FORMAT_R32G32_UINT;
            } break;
            case VertexFormat::Uint4: {
                result = VK_FORMAT_R32G32B32A32_UINT;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown vertex format");
            } break;
        }

        return result;
    }

    VkPrimitiveTopology to_vk(PrimitiveTopology topology) {
        VkPrimitiveTopology result = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        switch (topology) {
            case PrimitiveTopology::PointList: {
                result = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            } break;
            case PrimitiveTopology::LineList: {
                result = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            } break;
            case PrimitiveTopology::LineStrip: {
                result = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            } break;
            case PrimitiveTopology::TriangleList: {
                result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            } break;
            case PrimitiveTopology::TriangleStrip: {
                result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            } break;
            case PrimitiveTopology::TriangleFan: {
                result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown primitive topology");
            } break;
        }

        return result;
    }

    VkPolygonMode to_vk(PolygonMode mode) {
        VkPolygonMode result = VK_POLYGON_MODE_FILL;

        switch (mode) {
            case PolygonMode::Fill: {
                result = VK_POLYGON_MODE_FILL;
            } break;
            case PolygonMode::Line: {
                result = VK_POLYGON_MODE_LINE;
            } break;
            case PolygonMode::Point: {
                result = VK_POLYGON_MODE_POINT;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown polygon mode");
            } break;
        }

        return result;
    }

    VkCullModeFlags to_vk(CullMode mode) {
        VkCullModeFlags result = VK_CULL_MODE_NONE;

        switch (mode) {
            case CullMode::None: {
                result = VK_CULL_MODE_NONE;
            } break;
            case CullMode::Front: {
                result = VK_CULL_MODE_FRONT_BIT;
            } break;
            case CullMode::Back: {
                result = VK_CULL_MODE_BACK_BIT;
            } break;
            case CullMode::FrontAndBack: {
                result = VK_CULL_MODE_FRONT_AND_BACK;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown cull mode");
            } break;
        }

        return result;
    }

    VkFrontFace to_vk(FrontFace face) {
        VkFrontFace result = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        switch (face) {
            case FrontFace::CounterClockwise: {
                result = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            } break;
            case FrontFace::Clockwise: {
                result = VK_FRONT_FACE_CLOCKWISE;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown front face");
            } break;
        }

        return result;
    }

    VkCompareOp to_vk(CompareOp op) {
        VkCompareOp result = VK_COMPARE_OP_NEVER;

        switch (op) {
            case CompareOp::Never: {
                result = VK_COMPARE_OP_NEVER;
            } break;
            case CompareOp::Less: {
                result = VK_COMPARE_OP_LESS;
            } break;
            case CompareOp::Equal: {
                result = VK_COMPARE_OP_EQUAL;
            } break;
            case CompareOp::LessOrEqual: {
                result = VK_COMPARE_OP_LESS_OR_EQUAL;
            } break;
            case CompareOp::Greater: {
                result = VK_COMPARE_OP_GREATER;
            } break;
            case CompareOp::NotEqual: {
                result = VK_COMPARE_OP_NOT_EQUAL;
            } break;
            case CompareOp::GreaterOrEqual: {
                result = VK_COMPARE_OP_GREATER_OR_EQUAL;
            } break;
            case CompareOp::Always: {
                result = VK_COMPARE_OP_ALWAYS;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown compare op");
            } break;
        }

        return result;
    }

    VkStencilOp to_vk(StencilOp op) {
        VkStencilOp result = VK_STENCIL_OP_KEEP;

        switch (op) {
            case StencilOp::Keep: {
                result = VK_STENCIL_OP_KEEP;
            } break;
            case StencilOp::Zero: {
                result = VK_STENCIL_OP_ZERO;
            } break;
            case StencilOp::Replace: {
                result = VK_STENCIL_OP_REPLACE;
            } break;
            case StencilOp::IncrementAndClamp: {
                result = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            } break;
            case StencilOp::DecrementAndClamp: {
                result = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            } break;
            case StencilOp::Invert: {
                result = VK_STENCIL_OP_INVERT;
            } break;
            case StencilOp::IncrementAndWrap: {
                result = VK_STENCIL_OP_INCREMENT_AND_WRAP;
            } break;
            case StencilOp::DecrementAndWrap: {
                result = VK_STENCIL_OP_DECREMENT_AND_WRAP;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown stencil op");
            } break;
        }

        return result;
    }

    VkBlendFactor to_vk(BlendFactor factor) {
        VkBlendFactor result = VK_BLEND_FACTOR_ZERO;

        switch (factor) {
            case BlendFactor::Zero: {
                result = VK_BLEND_FACTOR_ZERO;
            } break;
            case BlendFactor::One: {
                result = VK_BLEND_FACTOR_ONE;
            } break;
            case BlendFactor::SrcColor: {
                result = VK_BLEND_FACTOR_SRC_COLOR;
            } break;
            case BlendFactor::OneMinusSrcColor: {
                result = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            } break;
            case BlendFactor::DstColor: {
                result = VK_BLEND_FACTOR_DST_COLOR;
            } break;
            case BlendFactor::OneMinusDstColor: {
                result = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            } break;
            case BlendFactor::SrcAlpha: {
                result = VK_BLEND_FACTOR_SRC_ALPHA;
            } break;
            case BlendFactor::OneMinusSrcAlpha: {
                result = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            } break;
            case BlendFactor::DstAlpha: {
                result = VK_BLEND_FACTOR_DST_ALPHA;
            } break;
            case BlendFactor::OneMinusDstAlpha: {
                result = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            } break;
            case BlendFactor::ConstantColor: {
                result = VK_BLEND_FACTOR_CONSTANT_COLOR;
            } break;
            case BlendFactor::OneMinusConstantColor: {
                result = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            } break;
            case BlendFactor::ConstantAlpha: {
                result = VK_BLEND_FACTOR_CONSTANT_ALPHA;
            } break;
            case BlendFactor::OneMinusConstantAlpha: {
                result = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            } break;
            case BlendFactor::SrcAlphaSaturate: {
                result = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            } break;
            case BlendFactor::Src1Color: {
                result = VK_BLEND_FACTOR_SRC1_COLOR;
            } break;
            case BlendFactor::OneMinusSrc1Color: {
                result = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            } break;
            case BlendFactor::Src1Alpha: {
                result = VK_BLEND_FACTOR_SRC1_ALPHA;
            } break;
            case BlendFactor::OneMinusSrc1Alpha: {
                result = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown blend factor");
            } break;
        }

        return result;
    }

    VkBlendOp to_vk(BlendOp op) {
        VkBlendOp result = VK_BLEND_OP_ADD;

        switch (op) {
            case BlendOp::Add: {
                result = VK_BLEND_OP_ADD;
            } break;
            case BlendOp::Subtract: {
                result = VK_BLEND_OP_SUBTRACT;
            } break;
            case BlendOp::ReverseSubtract: {
                result = VK_BLEND_OP_REVERSE_SUBTRACT;
            } break;
            case BlendOp::Min: {
                result = VK_BLEND_OP_MIN;
            } break;
            case BlendOp::Max: {
                result = VK_BLEND_OP_MAX;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown blend op");
            } break;
        }

        return result;
    }

    VkLogicOp to_vk(LogicOp op) {
        VkLogicOp result = VK_LOGIC_OP_CLEAR;

        switch (op) {
            case LogicOp::Clear: {
                result = VK_LOGIC_OP_CLEAR;
            } break;
            case LogicOp::And: {
                result = VK_LOGIC_OP_AND;
            } break;
            case LogicOp::AndReverse: {
                result = VK_LOGIC_OP_AND_REVERSE;
            } break;
            case LogicOp::Copy: {
                result = VK_LOGIC_OP_COPY;
            } break;
            case LogicOp::AndInverted: {
                result = VK_LOGIC_OP_AND_INVERTED;
            } break;
            case LogicOp::NoOp: {
                result = VK_LOGIC_OP_NO_OP;
            } break;
            case LogicOp::Xor: {
                result = VK_LOGIC_OP_XOR;
            } break;
            case LogicOp::Or: {
                result = VK_LOGIC_OP_OR;
            } break;
            case LogicOp::Nor: {
                result = VK_LOGIC_OP_NOR;
            } break;
            case LogicOp::Equivalent: {
                result = VK_LOGIC_OP_EQUIVALENT;
            } break;
            case LogicOp::Invert: {
                result = VK_LOGIC_OP_INVERT;
            } break;
            case LogicOp::OrReverse: {
                result = VK_LOGIC_OP_OR_REVERSE;
            } break;
            case LogicOp::CopyInverted: {
                result = VK_LOGIC_OP_COPY_INVERTED;
            } break;
            case LogicOp::OrInverted: {
                result = VK_LOGIC_OP_OR_INVERTED;
            } break;
            case LogicOp::Nand: {
                result = VK_LOGIC_OP_NAND;
            } break;
            case LogicOp::Set: {
                result = VK_LOGIC_OP_SET;
            } break;
            default: {
                MANTLE_FATAL(true, "Unknown logic op");
            } break;
        }

        return result;
    }

    VkColorComponentFlags to_vk(ColorWriteMask mask) {
        using U = std::underlying_type_t<ColorWriteMask>;
        U u = static_cast<U>(mask);

        if (mask == ColorWriteMask::None) {
            return 0;
        }

        if (mask == ColorWriteMask::RGBA) {
            return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                   VK_COLOR_COMPONENT_A_BIT;
        }

        VkColorComponentFlags flags = 0;

        auto has = [&](ColorWriteMask bit) { return (u & static_cast<U>(bit)) != 0; };

        if (has(ColorWriteMask::R)) {
            flags |= VK_COLOR_COMPONENT_R_BIT;
        }
        if (has(ColorWriteMask::G)) {
            flags |= VK_COLOR_COMPONENT_G_BIT;
        }
        if (has(ColorWriteMask::B)) {
            flags |= VK_COLOR_COMPONENT_B_BIT;
        }
        if (has(ColorWriteMask::A)) {
            flags |= VK_COLOR_COMPONENT_A_BIT;
        }

        return flags;
    }

    VkAccessFlags2 to_vk(AccessType access) {
        VkAccessFlags2 result = VK_ACCESS_2_NONE;

        switch (access) {
            case AccessType::None: {
                result = VK_ACCESS_2_NONE;
            } break;
            case AccessType::ColorAttachmentRead: {
                result = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
            } break;
            case AccessType::ColorAttachmentWrite: {
                result = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            } break;
            case AccessType::DepthAttachmentRead: {
                result = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            } break;
            case AccessType::DepthAttachmentWrite: {
                result = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            } break;
            case AccessType::ShaderRead: {
                result = VK_ACCESS_2_SHADER_READ_BIT;
            } break;
            case AccessType::ShaderWrite: {
                result = VK_ACCESS_2_SHADER_WRITE_BIT;
            } break;
            case AccessType::ShaderReadWrite: {
                result = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            } break;
            case AccessType::TransferRead: {
                result = VK_ACCESS_2_TRANSFER_READ_BIT;
            } break;
            case AccessType::TransferWrite: {
                result = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            } break;
            case AccessType::VertexAttributeRead: {
                result = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
            } break;
            case AccessType::IndexRead: {
                result = VK_ACCESS_2_INDEX_READ_BIT;
            } break;
            case AccessType::UniformRead: {
                result = VK_ACCESS_2_UNIFORM_READ_BIT;
            } break;
            case AccessType::IndirectCommandRead: {
                result = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
            } break;
            default: {
                MANTLE_FATAL(true, "unsupported AccessType");
            } break;
        }

        return result;
    }

} // namespace mantle
