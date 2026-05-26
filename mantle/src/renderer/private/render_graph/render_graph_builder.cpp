#include "renderer/render_graph.h"

namespace mantle {

    RGImageHandle RenderGraphBuilder::read(RGImageHandle image,
                                           ReadUsage usage) {
        m_image_reads->push_back({
            .pass_index = m_pass_index,
            .handle = image,
            .usage = usage,
        });
        return image;
    }

    RGImageHandle RenderGraphBuilder::write(RGImageHandle image,
                                            WriteUsage usage) {
        m_image_writes->push_back({
            .pass_index = m_pass_index,
            .handle = image,
            .usage = usage,
        });
        return image;
    }
} // namespace mantle
