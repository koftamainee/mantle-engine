// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "mantle/core/macros.h"
#include "mantle/core/types.h"

namespace mantle {

    class OSMemory final {
      public:
        OSMemory() = default;
        ~OSMemory();

        MANTLE_NO_COPY(OSMemory);

        void init();
        void destroy();

        void *reserve(usize size) const;
        void  commit(void *ptr, usize size) const;
        void  decommit(void *ptr, usize size) const;
        void  release(void *ptr, usize size) const;

        usize page_size() const;

      private:
        usize m_page_size = 0;
        bool  m_is_initialized = false;
    };

} // namespace mantle
