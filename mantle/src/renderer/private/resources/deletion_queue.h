#pragma once
#include <concepts>
#include <cstdint>
#include <functional>
#include <vector>

namespace mantle {
    class DeletionQueue {
    public:
        template <typename F>
        void push(F &&fn) {
            m_deletors.emplace_back(
                [fn = std::forward<F>(fn)]() mutable {
                    fn();
                }
                );
        }

        void flush() {
            for (auto &fn : m_deletors) {
                fn();
            }
            m_deletors.clear();
        }

    private:
        std::vector<std::function<void()>> m_deletors;
    };
}
