#pragma once

#include <vector>

#include "core/macros.h"
#include "renderer/types.h"


namespace mantle {
    class TransientResources final {
    public:
        TransientResources() = default;
        ~TransientResources() = default;

        MANTLE_NO_COPY_NO_MOVE(TransientResources);

        void set_imported_images(std::pmr::vector<ImageHandle> *images);
        void set_imported_buffers(std::pmr::vector<BufferHandle> *buffers);

        ImageHandle get_image(RGImageHandle handle);
        BufferHandle get_buffer(RGBufferHandle handle);

    private:
        std::pmr::vector<ImageHandle> *m_images = nullptr;
        std::pmr::vector<BufferHandle> *m_buffers = nullptr;
    };
} // namespace mantle
