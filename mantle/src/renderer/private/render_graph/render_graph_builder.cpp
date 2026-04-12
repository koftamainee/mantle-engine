#include "renderer/render_graph.h"

// TODO
namespace mantle {
    RenderGraphBuilder::~RenderGraphBuilder() {}

    RGImageHandle RenderGraphBuilder::create_image(const ImageDesc &desc) {}

    RGBufferHandle RenderGraphBuilder::create_buffer(const BufferDesc &desc) {}

    RGImageHandle RenderGraphBuilder::read(RGImageHandle image) {}

    RGImageHandle RenderGraphBuilder::write(RGImageHandle image) {}

    RGBufferHandle RenderGraphBuilder::read(RGBufferHandle buffer) {}

    RGBufferHandle RenderGraphBuilder::write(RGBufferHandle buffer) {}
} // namespace mantle
