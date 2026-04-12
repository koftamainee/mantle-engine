#include "renderer/render_graph.h"

#include "core/assert.h"

namespace mantle {
    RenderGraph::RenderGraph(ArenaAllocator *arena) :
        m_arena(arena), m_resource(arena), m_passes(&m_resource) {
        check(arena != nullptr);
        m_tag = arena->save();
    }
    RenderGraph::~RenderGraph() {
        m_passes.clear();
        m_arena->restore(m_tag);
    }

    CompiledRenderGraph RenderGraph::compile(GPUResourceManager &resources) {} // TODO
} // namespace mantle
