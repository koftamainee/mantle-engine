#include "core/memory/persistent_allocator.h"
#include "core/assert.h"

namespace mantle {

    void PersistentAllocator::init(VirtualHeap *heap) {
        MANTLE_CHECK(heap != nullptr);
        m_heap = heap;
    }

    MemoryBlock PersistentAllocator::take(usize size) {
        MANTLE_CHECK(m_heap != nullptr);
        return m_heap->take(size);
    }

} // namespace mantle