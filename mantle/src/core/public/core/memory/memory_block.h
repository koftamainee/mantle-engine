#pragma once

#include "core/types.h"

namespace mantle {

    struct MemoryBlock final {
        void *ptr;
        usize size;
    };

} // namespace mantle