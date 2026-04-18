#include "renderer/render_graph.h"

#include "core/assert.h"

namespace mantle {
    RenderGraph::RenderGraph(ArenaAllocator *arena) :
        m_arena(arena), m_scope(arena), m_resource(arena), m_passes(&m_resource) {
        check(arena != nullptr);
    }

    RGImageHandle RenderGraph::import_image(ImageHandle image) {
        return {}; // TODO
    } 

    RGBufferHandle RenderGraph::import_buffer(BufferHandle buffer) {
        return {}; // TODO
    }
} // namespace mantle
