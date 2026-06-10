// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <cstring>
#include <mutex>
#include <string_view>
#include <utility>

#include "core/macros.h"
#include "core/memory/allocator.h"
#include "core/memory/memory_block.h"
#include "core/types.h"

namespace mantle {

    template <typename M>
    concept SyncMechanism = requires(M m) {
        { m.lock() } -> std::same_as<void>;
        { m.unlock() } -> std::same_as<void>;
    };

    template <typename Inner, typename Mutex = std::mutex>
        requires CAllocator<Inner> && CTypedAllocator<Inner, u8> && CEmplaceAllocator<Inner, u8> &&
                 SyncMechanism<Mutex>
    class ThreadSafeAllocator final {
      public:
        ThreadSafeAllocator() = default;
        ~ThreadSafeAllocator() = default;

        MANTLE_NO_COPY(ThreadSafeAllocator);

        ThreadSafeAllocator(ThreadSafeAllocator &&other) noexcept {
            std::lock_guard lock(other.m_mutex);
            m_inner = std::move(other.m_inner);
        }

        ThreadSafeAllocator &operator=(ThreadSafeAllocator &&other) noexcept {
            if (this != &other) {
                std::scoped_lock lock(m_mutex, other.m_mutex);
                m_inner = std::move(other.m_inner);
            }
            return *this;
        }

        void init(MemoryBlock block, std::string_view debug_name = {}) {
            std::lock_guard lock(m_mutex);
            m_inner.init(block, debug_name);
        }

        void destroy() {
            std::lock_guard lock(m_mutex);
            m_inner.destroy();
        }

        [[nodiscard]] void *alloc(usize size, usize align = alignof(std::max_align_t)) {
            std::lock_guard lock(m_mutex);
            return m_inner.alloc(size, align);
        }

        [[nodiscard]] void *realloc(void *ptr, usize size) {
            std::lock_guard lock(m_mutex);
            return m_inner.realloc(ptr, size);
        }

        void free(void *ptr) {
            std::lock_guard lock(m_mutex);
            m_inner.free(ptr);
        }

        template <typename T>
        [[nodiscard]] T *alloc(usize count = 1) {
            std::lock_guard lock(m_mutex);
            return static_cast<T *>(m_inner.alloc(sizeof(T) * count, alignof(T)));
        }

        template <typename T>
        [[nodiscard]] T *alloc_zeroed(usize count = 1) {
            std::lock_guard lock(m_mutex);
            T              *ptr = static_cast<T *>(m_inner.alloc(sizeof(T) * count, alignof(T)));
            std::memset(ptr, 0, sizeof(T) * count);
            return ptr;
        }

        template <typename T, typename... Args>
        [[nodiscard]] T *emplace(Args &&...args) {
            std::lock_guard lock(m_mutex);
            void           *mem = m_inner.alloc(sizeof(T), alignof(T));
            return new (mem) T(std::forward<Args>(args)...);
        }

        void reset()
            requires CResetableAllocator<Inner>
        {
            m_inner.reset();
        }

      private:
        Inner m_inner;
        Mutex m_mutex;
    };

} // namespace mantle
