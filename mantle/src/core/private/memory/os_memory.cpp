#include "core/memory/os_memory.h"
#include "core/assert.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace mantle {

    void OSMemory::init() {
        check(!m_is_initialized);

#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        m_page_size = static_cast<usize>(info.dwPageSize);
#else
        m_page_size = static_cast<usize>(getpagesize());
#endif

        m_is_initialized = true;
    }

    void *OSMemory::reserve(usize size) const {
        check(m_is_initialized);
#ifdef _WIN32
        return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
#else
        void *ptr =
            mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return ptr == MAP_FAILED ? nullptr : ptr;
#endif
    }

    void OSMemory::commit(void *ptr, usize size) const {
        check(m_is_initialized);
#ifdef _WIN32
        VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
#else
        mprotect(ptr, size, PROT_READ | PROT_WRITE);
#endif
    }

    void OSMemory::decommit(void *ptr, usize size) const {
        check(m_is_initialized);
#ifdef _WIN32
        VirtualFree(ptr, size, MEM_DECOMMIT);
#else
        madvise(ptr, size, MADV_DONTNEED);
        mprotect(ptr, size, PROT_NONE);
#endif
    }

    void OSMemory::release(void *ptr, usize size) const {
        check(m_is_initialized);
#ifdef _WIN32
        VirtualFree(ptr, size, MEM_RELEASE);
#else
        munmap(ptr, size);
#endif
    }

    usize OSMemory::page_size() const {
        check(m_is_initialized);
        return m_page_size;
    }

} // namespace mantle
