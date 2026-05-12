#include "transient_resources.h"

#include "core/assert.h"

namespace mantle {
    TransientResources::~TransientResources() {
        destroy();
    }

    void TransientResources::init() {
        check(!m_is_initialized);

        spdlog::info("Transient resources system is initialized");
        m_is_initialized = true;
    }

    void TransientResources::destroy() {
        if (m_is_initialized) {
            spdlog::info("Transient resources system is destroyed");
            m_is_initialized = false;
        }
    }

    ImageHandle TransientResources::get_image(RGImageHandle handle) {
        // TODO
        MANTLE_TODO();
    }

    BufferHandle TransientResources::get_buffer(RGBufferHandle handle) {
        // TODO
        MANTLE_TODO();
    }
} // namespace mantle
