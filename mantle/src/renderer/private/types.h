#pragma once
#include "core/types.h"
#include "renderer/types.h"
#include "core/enum_flags.h"
#include <span>

namespace mantle {
    enum class ImageLayout : u8 {
        Undefined,
        General,
        ColorAttachment,
        DepthAttachment,
        ShaderReadOnly,
        TransferSrc,
        TransferDst,
        Present,
    };

    enum class PipelineStage : u32 {
        None = 0,
        Top = 1 << 0,
        Bottom = 1 << 1,
        AllCommands = 1 << 2,
        ComputeShader = 1 << 3,
        VertexShader = 1 << 4,
        FragmentShader = 1 << 5,
        ColorOutput = 1 << 6,
        EarlyDepth = 1 << 7,
        LateDepth = 1 << 8,
        Transfer = 1 << 9,
        MaxEnum = (1 << 10) - 1,
    };

    struct ImageBarrier final {
        ImageHandle image = {};
        ImageLayout from = ImageLayout::Undefined;
        ImageLayout to = ImageLayout::Undefined;
        PipelineStage src_stage = PipelineStage::None;
        PipelineStage dst_stage = PipelineStage::None;
    };

    struct BufferBarrier final {
        BufferHandle buffer = {};
        PipelineStage src_stage = PipelineStage::None;
        PipelineStage dst_stage = PipelineStage::None;
        VkAccessFlags2 src_access = 0;
        VkAccessFlags2 dst_access = 0;
    };

    struct ColorAttachment final {
        ImageHandle image = {};
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::Store;
        f32 clear_r = 0.0f;
        f32 clear_g = 0.0f;
        f32 clear_b = 0.0f;
        f32 clear_a = 1.0f;
    };

    struct DepthAttachment final {
        ImageHandle image = {};
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::DontCare;
        f32 clear_value = 1.0f;
    };

    struct RenderingInfo final {
        std::span<ColorAttachment> colors = {};
        DepthAttachment depth = {};
        u32 width = 0;
        u32 height = 0;
    };

    struct BufferCopyInfo final {
        BufferHandle src = {};
        BufferHandle dst = {};
        usize src_offset = 0;
        usize dst_offset = 0;
        usize size = 0;
    };

    struct BufferImageCopyInfo final {
        BufferHandle src = {};
        ImageHandle dst = {};
        usize buffer_offset = 0;
        u32 mip_level = 0;
    };

    using DrawInfo = RGDrawInfo;
    using DrawIndexedInfo = RGDrawIndexedInfo;
    using DispatchInfo = RGDispatchInfo;

} // namespace mantle
