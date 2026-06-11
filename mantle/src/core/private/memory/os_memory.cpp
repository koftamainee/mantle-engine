// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/core/memory/os_memory.h"
#include "mantle/core/assert.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace mantle {

    static inline bool is_aligned(void *ptr, usize align) {
        return (reinterpret_cast<uintptr_t>(ptr) & (align - 1)) == 0;
    }

    static inline bool is_aligned_size(usize size, usize align) {
        return (size & (align - 1)) == 0;
    }

    OSMemory::~OSMemory() { destroy(); }

    void OSMemory::destroy() { m_is_initialized = false; }

    void OSMemory::init() {
        MANTLE_CHECK(!m_is_initialized);

#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        m_page_size = static_cast<usize>(info.dwPageSize);
#else
        m_page_size = static_cast<usize>(getpagesize());
#endif

        MANTLE_FATAL((m_page_size == 0), "Invalid page size");

        MANTLE_FATAL((m_page_size & (m_page_size - 1)) != 0, "Page size is not power of two");

        m_is_initialized = true;
    }

    void *OSMemory::reserve(usize size) const {
        MANTLE_CHECK(m_is_initialized);

        MANTLE_FATAL(size == 0, "reserve size == 0");
        MANTLE_FATAL(!is_aligned_size(size, m_page_size), "reserve size not page aligned");

#ifdef _WIN32
        void *ptr = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
        MANTLE_FATAL(ptr == nullptr, "VirtualAlloc reserve failed");
        return ptr;
#else
        void *ptr = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        MANTLE_FATAL(ptr == MAP_FAILED, "mmap reserve failed");
        return ptr;
#endif
    }

    void OSMemory::commit(void *ptr, usize size) const {
        MANTLE_CHECK(m_is_initialized);

        MANTLE_FATAL(ptr == nullptr, "commit ptr == nullptr");
        MANTLE_FATAL(size == 0, "commit size == 0");

        MANTLE_FATAL(!is_aligned(ptr, m_page_size), "commit ptr not page aligned");
        MANTLE_FATAL(!is_aligned_size(size, m_page_size), "commit size not page aligned");

#ifdef _WIN32
        void *result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
        MANTLE_FATAL(result == nullptr, "VirtualAlloc commit failed");
#else
        int res = mprotect(ptr, size, PROT_READ | PROT_WRITE);
        MANTLE_FATAL(res != 0, "mprotect commit failed");
#endif
    }

    void OSMemory::decommit(void *ptr, usize size) const {
        MANTLE_CHECK(m_is_initialized);

        MANTLE_FATAL(ptr == nullptr, "decommit ptr == nullptr");
        MANTLE_FATAL(size == 0, "decommit size == 0");

        MANTLE_FATAL(!is_aligned(ptr, m_page_size), "decommit ptr not page aligned");
        MANTLE_FATAL(!is_aligned_size(size, m_page_size), "decommit size not page aligned");

#ifdef _WIN32
        BOOL ok = VirtualFree(ptr, size, MEM_DECOMMIT);
        MANTLE_FATAL(!ok, "VirtualFree decommit failed");
#else
        int r1 = madvise(ptr, size, MADV_DONTNEED);
        MANTLE_FATAL(r1 != 0, "madvise failed");

        int r2 = mprotect(ptr, size, PROT_NONE);
        MANTLE_FATAL(r2 != 0, "mprotect decommit failed");
#endif
    }

    void OSMemory::release(void *ptr, usize size) const {
        MANTLE_CHECK(m_is_initialized);

        MANTLE_FATAL(ptr == nullptr, "release ptr == nullptr");
        MANTLE_FATAL(size == 0, "release size == 0");

        MANTLE_FATAL(!is_aligned(ptr, m_page_size), "release ptr not page aligned");
        MANTLE_FATAL(!is_aligned_size(size, m_page_size), "release size not page aligned");

#ifdef _WIN32
        BOOL ok = VirtualFree(ptr, 0, MEM_RELEASE);
        MANTLE_FATAL(!ok, "VirtualFree release failed");
#else
        int res = munmap(ptr, size);
        MANTLE_FATAL(res != 0, "munmap failed");
#endif
    }

    usize OSMemory::page_size() const {
        MANTLE_CHECK(m_is_initialized);
        return m_page_size;
    }

} // namespace mantle
