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
        m_committed = 0;
        m_is_initialized = true;
    }

    void VirtualHeap::destroy() {
        if (m_is_initialized) {
            m_os->release(m_base, m_reserved);

            m_os = nullptr;
            m_base = nullptr;
            m_reserved = 0;
            m_used = 0;
            m_committed = 0;
            m_is_initialized = false;
        }
    }

    MemoryBlock VirtualHeap::take(usize size) {
        check(m_is_initialized);
        check(size > 0);
        fatal(m_used + size > m_reserved, "Out of memory");

        void *ptr = static_cast<u8 *>(m_base) + m_used;

        const usize page = m_os->page_size();
        usize commit_end = (m_used + size + page - 1) & ~(page - 1);

        if (commit_end > m_committed) {
            void *commit_ptr = static_cast<u8 *>(m_base) + m_committed;
            m_os->commit(commit_ptr, commit_end - m_committed);
            m_committed = commit_end;
        }

        m_used += size;
        return {ptr, size};
    }

    usize VirtualHeap::reserved() const {
        check(m_is_initialized);
        return m_reserved;
    }

    usize VirtualHeap::used() const {
        check(m_is_initialized);
        return m_used;
    }

    usize VirtualHeap::committed() const {
        check(m_is_initialized);
        return m_committed;
    }

} // namespace mantle