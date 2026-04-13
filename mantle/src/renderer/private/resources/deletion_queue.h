#pragma once
#include <functional>
#include <vector>

namespace mantle {
    class DeletionQueue {
      public:
        template <typename F>
        void push(F &&fn) {
            m_deletors.emplace_back(
                [fn = std::forward<F>(fn)]() mutable { fn(); });
        }

        void flush() {
            for (auto &fn : m_deletors) {
                fn();
            }
            m_deletors.clear();
        }

      private:
        // this is kinda bad for memory allocation, but i just dont want to
        // optimize it yet
        std::vector<std::function<void()>> m_deletors;
    };
} // namespace mantle
