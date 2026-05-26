#include "renderer/render_graph.h"

#include "core/assert.h"
#include "render_graph/render_graph_internal.h"

namespace mantle {
    RenderGraph::RenderGraph(ArenaAllocator *arena) :
        m_arena(arena), m_scope(arena), m_resource(arena), m_passes(&m_resource),
        m_imported_images(&m_resource), m_imported_buffers(&m_resource),
        m_image_reads(&m_resource), m_image_writes(&m_resource) {
        check(arena != nullptr);
    }

    RGImageHandle RenderGraph::import_image(ImageHandle image) {
        RGImageHandle handle;
        handle.index = m_next_image_index++;
        handle.generation = 1;

        if (handle.index >= m_imported_images.size()) {
            m_imported_images.resize(handle.index + 1);
        }
        m_imported_images[handle.index] = image;

        return handle;
    }

    RGBufferHandle RenderGraph::import_buffer(BufferHandle buffer) {
        RGBufferHandle handle;
        handle.index = m_next_buffer_index++;
        handle.generation = 1;

        if (handle.index >= m_imported_buffers.size()) {
            m_imported_buffers.resize(handle.index + 1);
        }
        m_imported_buffers[handle.index] = buffer;

        return handle;
    }
} // namespace mantle
