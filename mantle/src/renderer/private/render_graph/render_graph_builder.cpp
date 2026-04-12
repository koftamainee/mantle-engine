#include "renderer/render_graph.h"

// TODO
namespace mantle {
    RenderGraphBuilder::~RenderGraphBuilder() {}

    RGImageHandle RenderGraphBuilder::create_image(const ImageDesc &desc) {}

    RGBufferHandle RenderGraphBuilder::create_buffer(const BufferDesc &desc) {}

    RGImageHandle RenderGraphBuilder::import_image(ImageHandle image) {}

    RGBufferHandle RenderGraphBuilder::import_buffer(BufferHandle buffer) {}

    RGImageHandle RenderGraphBuilder::read(RGImageHandle image) {}

    RGImageHandle RenderGraphBuilder::write(RGImageHandle image) {}

    RGBufferHandle RenderGraphBuilder::read(RGBufferHandle buffer) {}

    RGBufferHandle RenderGraphBuilder::write(RGBufferHandle buffer) {}
} // namespace mantle
