// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <vector>

#include "mantle/core/macros.h"
#include "mantle/core/memory/pmr/tlsf_resource.h"
#include "mantle/renderer/types.h"

namespace mantle {
    class GPUResourceManager;

    class TransientResources final {
      public:
        TransientResources() = default;
        ~TransientResources() = default;

        MANTLE_NO_COPY_NO_MOVE(TransientResources);

        void init(TlsfResource *persistent_resource, GPUResourceManager *resource_manager);

        void set_images(std::pmr::vector<FGImageEntry> *images);
        void set_buffers(std::pmr::vector<FGBufferEntry> *buffers);

        ImageHandle  get_image(FGImageHandle handle);
        BufferHandle get_buffer(FGBufferHandle handle);

      private:
        struct CreatedImageEntry {
            ImageHandle handle;
            ImageDesc   desc;
        };

        struct CreatedBufferEntry {
            BufferHandle handle;
            BufferDesc   desc;
        };

        std::pmr::vector<FGImageEntry>  *m_images = nullptr;
        std::pmr::vector<FGBufferEntry> *m_buffers = nullptr;

        GPUResourceManager                  *m_resource_manager = nullptr;
        std::pmr::vector<CreatedImageEntry>  m_image_cache;
        std::pmr::vector<CreatedBufferEntry> m_buffer_cache;
    };
} // namespace mantle
