// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/renderer/frame_graph.h"

namespace mantle {

    FGImageHandle FrameGraphBuilder::create_image(const ImageDesc &desc) {
        u32 index = (*m_next_image_index)++;
        if (index >= m_images->size()) {
            m_images->resize(index + 1);
        }
        (*m_images)[index] = FGImageEntry {.desc = desc};

        FGImageHandle handle;
        handle.index = index;
        handle.generation = 1;
        return handle;
    }

    FGBufferHandle FrameGraphBuilder::create_buffer(const BufferDesc &desc) {
        u32 index = (*m_next_buffer_index)++;
        if (index >= m_buffers->size()) {
            m_buffers->resize(index + 1);
        }
        (*m_buffers)[index] = FGBufferEntry {.desc = desc};

        FGBufferHandle handle;
        handle.index = index;
        handle.generation = 1;
        return handle;
    }

    FGImageHandle FrameGraphBuilder::read(FGImageHandle image, ReadUsage usage) {
        m_image_reads->push_back({
            .pass_index = m_pass_index,
            .handle = image,
            .usage = usage,
        });

        return image;
    }

    FGImageHandle FrameGraphBuilder::write(FGImageHandle image, WriteUsage usage) {
        m_image_writes->push_back({
            .pass_index = m_pass_index,
            .handle = image,
            .usage = usage,
        });

        auto &entry = (*m_images)[image.index];
        entry.version++;

        FGImageHandle new_handle;
        new_handle.index = image.index;
        new_handle.generation = entry.version;
        return new_handle;
    }

    FGBufferHandle FrameGraphBuilder::read(FGBufferHandle buffer, BufferReadUsage usage) {
        m_buffer_reads->push_back({
            .pass_index = m_pass_index,
            .handle = buffer,
            .usage = usage,
        });
        return buffer;
    }

    FGBufferHandle FrameGraphBuilder::write(FGBufferHandle buffer, BufferWriteUsage usage) {
        m_buffer_writes->push_back({
            .pass_index = m_pass_index,
            .handle = buffer,
            .usage = usage,
        });

        auto &entry = (*m_buffers)[buffer.index];
        entry.version++;

        FGBufferHandle new_handle;
        new_handle.index = buffer.index;
        new_handle.generation = entry.version;
        return new_handle;
    }
} // namespace mantle
