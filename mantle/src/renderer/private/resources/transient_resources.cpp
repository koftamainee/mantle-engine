#include "transient_resources.h"

#include "core/assert.h"

namespace mantle {
    void TransientResources::set_imported_images(
        std::pmr::vector<ImageHandle> *images) {
        m_images = images;
    }

    void TransientResources::set_imported_buffers(
        std::pmr::vector<BufferHandle> *buffers) {
        m_buffers = buffers;
    }

    ImageHandle TransientResources::get_image(RGImageHandle handle) {
        check(m_images != nullptr);
        check(handle.is_valid());
        check(handle.index < m_images->size());

        return (*m_images)[handle.index];
    }

    BufferHandle TransientResources::get_buffer(RGBufferHandle handle) {
        check(m_buffers != nullptr);
        check(handle.is_valid());
        check(handle.index < m_buffers->size());

        return (*m_buffers)[handle.index];
    }
} // namespace mantle
