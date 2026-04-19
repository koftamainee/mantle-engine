#pragma once
#include <span>
#include "core/enum_flags.h"
#include "core/types.h"
#include "renderer/types.h"

namespace mantle {
    struct ImageResource;
    struct BufferResource;

    enum class ImageLayout {
        Undefined,
        General,
        ColorAttachment,
        DepthAttachment,
        ShaderReadOnly,
        TransferSrc,
        TransferDst,
        Present,
    };

    enum class PipelineStage {
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
        VertexInput = 1 << 10,
        DrawIndirect = 1 << 11,
        Host = 1 << 12,
        MaxEnum = (1 << 13) - 1,
    };

    enum class AccessType {
        None,

        ColorAttachmentRead,
        ColorAttachmentWrite,
        DepthAttachmentRead,
        DepthAttachmentWrite,

        ShaderRead,
        ShaderWrite,
        ShaderReadWrite,

        TransferRead,
        TransferWrite,
    };

    struct ImageBarrier final {
        ImageResource *image = nullptr;
        ImageLayout from = ImageLayout::Undefined;
        ImageLayout to = ImageLayout::Undefined;
        PipelineStage src_stage = PipelineStage::None;
        PipelineStage dst_stage = PipelineStage::None;
        AccessType src_access = AccessType::None;
        AccessType dst_access = AccessType::None;
    };

    struct BufferBarrier final {
        BufferResource *buffer = nullptr;
        PipelineStage src_stage = PipelineStage::None;
        PipelineStage dst_stage = PipelineStage::None;
        AccessType src_access = AccessType::None;
        AccessType dst_access = AccessType::None;
    };

    struct ColorAttachment final {
        ImageResource *image = nullptr;
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::Store;
        f32 clear_r = 0.0f;
        f32 clear_g = 0.0f;
        f32 clear_b = 0.0f;
        f32 clear_a = 1.0f;
    };

    struct DepthAttachment final {
        ImageResource *image = nullptr;
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::DontCare;
        f32 clear_value = 1.0f;
    };

    struct RenderingInfo final {
        std::span<ColorAttachment> colors = {};
        DepthAttachment *depth = nullptr;
        u32 width = 0;
        u32 height = 0;
    };

    struct BufferCopyInfo final {
        BufferResource *src = nullptr;
        BufferResource *dst = nullptr;
        usize src_offset = 0;
        usize dst_offset = 0;
        usize size = 0;
    };

    struct BufferImageCopyInfo final {
        BufferResource *src = nullptr;
        ImageResource *dst = nullptr;
        usize buffer_offset = 0;
        u32 mip_level = 0;
    };

    using DrawInfo = RGDrawInfo;
    using DrawIndexedInfo = RGDrawIndexedInfo;
    using DispatchInfo = RGDispatchInfo;


    struct ImageBufferCopyInfo final {
        ImageResource *src = nullptr;
        BufferResource *dst = nullptr;
        usize buffer_offset = 0;
        u32 mip_level = 0;
    };

    struct ImageCopyInfo final {
        ImageResource *src = nullptr;
        ImageResource *dst = nullptr;
        u32 src_mip_level = 0;
        u32 dst_mip_level = 0;
        u32 src_array_layer = 0;
        u32 dst_array_layer = 0;

        // 0 = entire mip of src
        u32 width = 0;
        u32 height = 0;
    };

    struct ImageRegion final {
        u32 mip_level = 0;
        u32 array_layer = 0;
        i32 offset_x = 0;
        i32 offset_y = 0;
        u32 width = 0;
        u32 height = 0;
    };

    struct ImageBlitInfo final {
        ImageResource *src = nullptr;
        ImageResource *dst = nullptr;
        ImageRegion src_region = {};
        ImageRegion dst_region = {};
        Filter filter = Filter::Linear;
    };

} // namespace mantle
