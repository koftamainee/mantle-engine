#pragma once
#include <oneapi/tbb/detail/_template_helpers.h>
#include <ranges>
#include <vector>

#include "core/memory/arena_allocator.h"
#include "core/memory/memory_units.h"
#include "core/memory/pmr/arena_resource.h"

namespace mantle {
    template <typename F>
    concept CDeletable = requires(F f) {
        { f() } -> std::same_as<void>;
    };

    class DeletionQueue {
      public:
        DeletionQueue() = default;
        ~DeletionQueue() { flush(); }

        MANTLE_NO_COPY_NO_MOVE(DeletionQueue);

        void init(VirtualHeap *heap) {
            m_arena.init(heap->take(kilobytes(256)));
            m_resource = ArenaResource(&m_arena);
            m_deletors = std::pmr::vector<Deletor>(&m_resource);
        }

        template <typename T>
        void push(T *ptr) {
            m_deletors.push_back(
                {[](void *p) { static_cast<T *>(p)->~T(); }, ptr});
        }

        template <typename F>
            requires CDeletable<F>
        void push_fn(F &&fn) {
            using Fn = std::decay_t<F>;

            Fn *f = m_arena.emplace<Fn>(std::forward<F>(fn));

            m_deletors.push_back({[](void *p) {
                                      auto func = static_cast<Fn *>(p);
                                      (*func)();
                                      func->~Fn();
                                  },
                                  f});
        }

        void flush() {
            for (auto &m_deletor : std::views::reverse(m_deletors)) {
                m_deletor.call(m_deletor.data);
            }
            m_deletors.clear();
            m_deletors.shrink_to_fit();
            m_arena.reset();
        }

      private:
        struct Deletor {
            void (*call)(void *);
            void *data;
        };

        std::pmr::vector<Deletor> m_deletors;
        ArenaAllocator m_arena;
        ArenaResource m_resource;
    };
} // namespace mantle
