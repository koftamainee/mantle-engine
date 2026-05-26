#pragma once

#include "core/assert.h"
#include "types.h"

namespace mantle {

inline ImageLayout write_usage_to_layout(WriteUsage usage) {
    switch (usage) {
    case WriteUsage::ColorAttachment:
        return ImageLayout::ColorAttachment;
    case WriteUsage::DepthAttachment:
        return ImageLayout::DepthAttachment;
    case WriteUsage::StorageWrite:
        return ImageLayout::General;
    case WriteUsage::TransferDst:
        return ImageLayout::TransferDst;
    case WriteUsage::Clear:
        return ImageLayout::General;
    case WriteUsage::Present:
        return ImageLayout::Present;
    default:
        fatal(true, "unknown WriteUsage");
    }
}

inline PipelineStage write_usage_to_stage(WriteUsage usage) {
    switch (usage) {
    case WriteUsage::ColorAttachment:
        return PipelineStage::ColorOutput;
    case WriteUsage::DepthAttachment:
        return PipelineStage::EarlyDepth;
    case WriteUsage::StorageWrite:
        return PipelineStage::ComputeShader;
    case WriteUsage::TransferDst:
        return PipelineStage::Transfer;
    case WriteUsage::Clear:
        return PipelineStage::Top;
    case WriteUsage::Present:
        return PipelineStage::Bottom;
    default:
        fatal(true, "unknown WriteUsage");
    }
}

inline AccessType write_usage_to_access(WriteUsage usage) {
    switch (usage) {
    case WriteUsage::ColorAttachment:
        return AccessType::ColorAttachmentWrite;
    case WriteUsage::DepthAttachment:
        return AccessType::DepthAttachmentWrite;
    case WriteUsage::StorageWrite:
        return AccessType::ShaderWrite;
    case WriteUsage::TransferDst:
        return AccessType::TransferWrite;
    case WriteUsage::Clear:
        return AccessType::None;
    case WriteUsage::Present:
        return AccessType::None;
    default:
        fatal(true, "unknown WriteUsage");
    }
}

inline ImageLayout read_usage_to_layout(ReadUsage usage) {
    switch (usage) {
    case ReadUsage::Sampled:
        return ImageLayout::ShaderReadOnly;
    case ReadUsage::StorageRead:
        return ImageLayout::General;
    case ReadUsage::InputAttachment:
        return ImageLayout::ShaderReadOnly;
    case ReadUsage::TransferSrc:
        return ImageLayout::TransferSrc;
    case ReadUsage::IndirectArg:
        return ImageLayout::ShaderReadOnly;
    default:
        fatal(true, "unknown ReadUsage");
    }
}

inline PipelineStage read_usage_to_stage(ReadUsage usage) {
    switch (usage) {
    case ReadUsage::Sampled:
        return PipelineStage::FragmentShader;
    case ReadUsage::StorageRead:
        return PipelineStage::ComputeShader;
    case ReadUsage::InputAttachment:
        return PipelineStage::FragmentShader;
    case ReadUsage::TransferSrc:
        return PipelineStage::Transfer;
    case ReadUsage::IndirectArg:
        return PipelineStage::DrawIndirect;
    default:
        fatal(true, "unknown ReadUsage");
    }
}

inline AccessType read_usage_to_access(ReadUsage usage) {
    switch (usage) {
    case ReadUsage::Sampled:
        return AccessType::ShaderRead;
    case ReadUsage::StorageRead:
        return AccessType::ShaderRead;
    case ReadUsage::InputAttachment:
        return AccessType::ShaderRead;
    case ReadUsage::TransferSrc:
        return AccessType::TransferRead;
    case ReadUsage::IndirectArg:
        return AccessType::None;
    default:
        fatal(true, "unknown ReadUsage");
    }
}

} // namespace mantle
