#include "core/memory/persistent_allocator.h"
#include "core/assert.h"

namespace mantle {

    void PersistentAllocator::init(VirtualHeap *heap) {
        check(heap != nullptr);
        m_heap = heap;
    }

    MemoryBlock PersistentAllocator::take(usize size) {
        check(m_heap != nullptr);
        return m_heap->take(size);
    }

} // namespace mantle