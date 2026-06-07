#include "core/concurrency/worker_pool.h"
#include "core/assert.h"

namespace mantle {

    WorkerPool::~WorkerPool() { destroy(); }

    void WorkerPool::init(u32 num_workers, usize scratch_size,
                          VirtualHeap *heap) {
        MANTLE_CHECK(m_workers.empty());

        m_workers.resize(num_workers);
        for (u32 i = 0; i < num_workers; i++) {
            auto &w = m_workers[i];
            MemoryBlock mem = heap->take(scratch_size);
            w.scratch.init(mem);
            w.thread = std::jthread([this, &w](std::stop_token stop) {
                worker_loop(w, stop);
            });
        }
    }

    void WorkerPool::destroy() {
        {
            std::lock_guard lock(m_mutex);
            m_stop = true;
        }
        m_cv_has_work.notify_all();

        for (auto &w : m_workers) {
            if (w.thread.joinable()) {
                w.thread.request_stop();
                w.thread.join();
            }
        }
        m_workers.clear();
    }

    void WorkerPool::submit(TaskFn fn, void *user_data) {
        {
            std::lock_guard lock(m_mutex);
            m_queue.push({fn, user_data});
            m_in_flight++;
        }
        m_cv_has_work.notify_one();
    }

    void WorkerPool::wait() {
        std::unique_lock lock(m_mutex);
        m_cv_done.wait(lock, [this] { return m_in_flight == 0; });
    }

    void WorkerPool::worker_loop(Worker &worker, std::stop_token stop) {
        while (!stop.stop_requested()) {
            std::pair<TaskFn, void *> task;
            {
                std::unique_lock lock(m_mutex);
                m_cv_has_work.wait(lock, [this, &stop] {
                    return !m_queue.empty() || m_stop || stop.stop_requested();
                });
                if (m_queue.empty() || stop.stop_requested())
                    return;
                task = m_queue.front();
                m_queue.pop();
            }
            task.first(worker.scratch, task.second);
            {
                std::lock_guard lock(m_mutex);
                m_in_flight--;
            }
            m_cv_done.notify_one();
        }
    }

} // namespace mantle
