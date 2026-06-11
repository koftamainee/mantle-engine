// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "mantle/core/macros.h"
#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/types.h"

namespace mantle {

    class WorkerPool final {
      public:
        WorkerPool() = default;
        ~WorkerPool();

        MANTLE_NO_COPY_NO_MOVE(WorkerPool);

        void init(u32 num_workers, usize scratch_size, MemoryBlock block);
        void destroy();

        using TaskFn = void (*)(ArenaAllocator &scratch, void *user_data);
        void submit(TaskFn fn, void *user_data);
        void wait();

        u32 num_workers() const { return static_cast<u32>(m_workers.size()); }

      private:
        struct Worker {
            std::jthread   thread;
            ArenaAllocator scratch;
        };

        void worker_loop(Worker &worker, std::stop_token stop);

        std::vector<Worker>                   m_workers;
        std::mutex                            m_mutex;
        std::condition_variable               m_cv_has_work;
        std::condition_variable               m_cv_done;
        std::queue<std::pair<TaskFn, void *>> m_queue;
        u32                                   m_in_flight = 0;
        bool                                  m_stop = false;
    };

} // namespace mantle
