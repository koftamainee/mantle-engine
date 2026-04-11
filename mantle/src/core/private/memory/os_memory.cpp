#include "core/memory/os_memory.h"
#include "core/assert.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace mantle {

    static inline bool is_aligned(void* ptr, usize align) {
        return (reinterpret_cast<uintptr_t>(ptr) & (align - 1)) == 0;
    }

    static inline bool is_aligned_size(usize size, usize align) {
        return (size & (align - 1)) == 0;
    }

    OSMemory::~OSMemory() { destroy(); }

    void OSMemory::destroy() { m_is_initialized = false; }

    void OSMemory::init() {
        check(!m_is_initialized);

#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        m_page_size = static_cast<usize>(info.dwPageSize);
#else
        m_page_size = static_cast<usize>(getpagesize());
#endif

        fatal((m_page_size == 0), "Invalid page size");

        fatal((m_page_size & (m_page_size - 1)) != 0,
              "Page size is not power of two");

        m_is_initialized = true;
    }

    void* OSMemory::reserve(usize size) const {
        check(m_is_initialized);

        fatal(size == 0, "reserve size == 0");
        fatal(!is_aligned_size(size, m_page_size),
              "reserve size not page aligned");

#ifdef _WIN32
        void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
        fatal(ptr == nullptr, "VirtualAlloc reserve failed");
        return ptr;
#else
        void* ptr = mmap(nullptr, size, PROT_NONE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        fatal(ptr == MAP_FAILED, "mmap reserve failed");
        return ptr;
#endif
    }

    void OSMemory::commit(void* ptr, usize size) const {
        check(m_is_initialized);

        fatal(ptr == nullptr, "commit ptr == nullptr");
        fatal(size == 0, "commit size == 0");

        fatal(!is_aligned(ptr, m_page_size),
              "commit ptr not page aligned");
        fatal(!is_aligned_size(size, m_page_size),
              "commit size not page aligned");

#ifdef _WIN32
        void* result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
        fatal(result == nullptr, "VirtualAlloc commit failed");
#else
        int res = mprotect(ptr, size, PROT_READ | PROT_WRITE);
        fatal(res != 0, "mprotect commit failed");
#endif
    }

    void OSMemory::decommit(void* ptr, usize size) const {
        check(m_is_initialized);

        fatal(ptr == nullptr, "decommit ptr == nullptr");
        fatal(size == 0, "decommit size == 0");

        fatal(!is_aligned(ptr, m_page_size),
              "decommit ptr not page aligned");
        fatal(!is_aligned_size(size, m_page_size),
              "decommit size not page aligned");

#ifdef _WIN32
        BOOL ok = VirtualFree(ptr, size, MEM_DECOMMIT);
        fatal(!ok, "VirtualFree decommit failed");
#else
        int r1 = madvise(ptr, size, MADV_DONTNEED);
        fatal(r1 != 0, "madvise failed");

        int r2 = mprotect(ptr, size, PROT_NONE);
        fatal(r2 != 0, "mprotect decommit failed");
#endif
    }

    void OSMemory::release(void* ptr, usize size) const {
        check(m_is_initialized);

        fatal(ptr == nullptr, "release ptr == nullptr");
        fatal(size == 0, "release size == 0");

        fatal(!is_aligned(ptr, m_page_size),
              "release ptr not page aligned");
        fatal(!is_aligned_size(size, m_page_size),
              "release size not page aligned");

#ifdef _WIN32
        BOOL ok = VirtualFree(ptr, 0, MEM_RELEASE);
        fatal(!ok, "VirtualFree release failed");
#else
        int res = munmap(ptr, size);
        fatal(res != 0, "munmap failed");
#endif
    }

    usize OSMemory::page_size() const {
        check(m_is_initialized);
        return m_page_size;
    }

} // namespace mantle
