#include "renderer/frame_graph.h"

#include "core/assert.h"
#include "frame_graph/render_graph_internal.h"

namespace mantle {
    FrameGraph::FrameGraph(ArenaAllocator *arena) :
        m_arena(arena), m_scope(arena), m_resource(arena),
        m_blackboard(&m_resource), m_passes(&m_resource),
        m_images(&m_resource), m_buffers(&m_resource),
        m_image_reads(&m_resource), m_image_writes(&m_resource),
        m_buffer_reads(&m_resource), m_buffer_writes(&m_resource),
        m_next_image_index(0), m_next_buffer_index(0) {
        MANTLE_CHECK(arena != nullptr);
    }

    FGImageHandle FrameGraph::import_image(ImageHandle image) {
        FGImageHandle handle;
        handle.index = m_next_image_index++;
        handle.generation = 1;

        if (handle.index >= m_images.size()) {
            m_images.resize(handle.index + 1);
        }
        m_images[handle.index] = FGImageEntry{.imported_handle = image};

        return handle;
    }

    FGBufferHandle FrameGraph::import_buffer(BufferHandle buffer) {
        FGBufferHandle handle;
        handle.index = m_next_buffer_index++;
        handle.generation = 1;

        if (handle.index >= m_buffers.size()) {
            m_buffers.resize(handle.index + 1);
        }
        m_buffers[handle.index] = FGBufferEntry{.imported_handle = buffer};

        return handle;
    }
} // namespace mantle
