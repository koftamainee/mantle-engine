// Copyright (c) 2026 Mantle. All rights reserved.

#include "sdl_allocator.h"

#include <SDL3/SDL_stdinc.h>

#include <new>

#include "mantle/core/assert.h"

namespace mantle {

    static SDLAllocator *s_instance = nullptr;
    alignas(alignof(SDLAllocator)) static std::byte s_storage[sizeof(SDLAllocator)];

    SDLAllocator &SDLAllocator::create(MemoryBlock block) {
        MANTLE_CHECK(s_instance == nullptr);
        s_instance = new (s_storage) SDLAllocator(block);
        return *s_instance;
    }

    SDLAllocator &SDLAllocator::instance() {
        MANTLE_CHECK(s_instance != nullptr);
        return *s_instance;
    }

    void SDLAllocator::destroy() {
        MANTLE_CHECK(s_instance != nullptr);
        s_instance->~SDLAllocator();
        s_instance = nullptr;
    }

    SDLAllocator::SDLAllocator(MemoryBlock block) :
        m_malloc(sdl_malloc_impl),
        m_calloc(sdl_calloc_impl),
        m_realloc(sdl_realloc_impl),
        m_free(sdl_free_impl) {
        m_tlsf.init(block);
    }

    SDLAllocator::~SDLAllocator() { m_tlsf.destroy(); }


    SDL_malloc_func  SDLAllocator::malloc() const { return m_malloc; }
    SDL_calloc_func  SDLAllocator::calloc() const { return m_calloc; }
    SDL_realloc_func SDLAllocator::realloc() const { return m_realloc; }
    SDL_free_func    SDLAllocator::free() const { return m_free; }


    void *SDLCALL SDLAllocator::sdl_malloc_impl(size_t size) {
        return instance().m_tlsf.alloc(size);
    }

    void *SDLCALL SDLAllocator::sdl_calloc_impl(size_t nmemb, size_t size) {
        const usize total = nmemb * size;

        void *ptr = instance().m_tlsf.alloc(total);
        if (ptr != nullptr) {
            SDL_memset(ptr, 0, total);
        }
        return ptr;
    }

    void *SDLCALL SDLAllocator::sdl_realloc_impl(void *mem, size_t size) {
        return instance().m_tlsf.realloc(mem, size);
    }

    void SDLCALL SDLAllocator::sdl_free_impl(void *mem) {
        if (mem != nullptr) {
            instance().m_tlsf.free(mem);
        }
    }

} // namespace mantle
