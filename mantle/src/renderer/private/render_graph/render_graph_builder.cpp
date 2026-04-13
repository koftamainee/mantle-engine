#include "renderer/render_graph.h"

// TODO
namespace mantle {
    RenderGraphBuilder::~RenderGraphBuilder() {}

    RGImageHandle RenderGraphBuilder::create_image(const ImageDesc &desc) {
        return {};
    }

    RGBufferHandle RenderGraphBuilder::create_buffer(const BufferDesc &desc) {
        return {};
     }

    RGImageHandle RenderGraphBuilder::read(RGImageHandle image) { return {}; }

    RGImageHandle RenderGraphBuilder::write(RGImageHandle image) { return {}; }

    RGBufferHandle RenderGraphBuilder::read(RGBufferHandle buffer) {
        return {};
    }

    RGBufferHandle RenderGraphBuilder::write(RGBufferHandle buffer) {
        return {};
    }
} // namespace mantle
