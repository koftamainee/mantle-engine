#include "core/memory/virtual_heap.h"
#include "core/assert.h"

namespace mantle {

    VirtualHeap::~VirtualHeap() { destroy(); }

    void VirtualHeap::init(OSMemory &os, usize reserve_size) {
        check(!m_is_initialized);
        check(reserve_size > 0);

        m_os = &os;
        m_base = m_os->reserve(reserve_size);
        check(m_base != nullptr);

        m_reserved = reserve_size;
        m_used = 0;
        m_is_initialized = true;
    }

    void VirtualHeap::destroy() {
        if (m_is_initialized) {

            m_os->release(m_base, m_reserved);

            m_os = nullptr;
            m_base = nullptr;
            m_reserved = 0;
            m_used = 0;
            m_is_initialized = false;
        }
    }

    void *VirtualHeap::take(usize size) {
        check(m_is_initialized);
        check(m_used + size <= m_reserved);

        void *ptr = static_cast<u8 *>(m_base) + m_used;
        m_used += size;
        return ptr;
    }

    usize VirtualHeap::reserved() const {
        check(m_is_initialized);
        return m_reserved;
    }

    usize VirtualHeap::used() const {
        check(m_is_initialized);
        return m_used;
    }

} // namespace mantle
