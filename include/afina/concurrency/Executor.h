#ifndef AFINA_CONCURRENCY_EXECUTOR_H
#define AFINA_CONCURRENCY_EXECUTOR_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <iostream>

namespace Afina {
namespace Concurrency {

/**
 * # Thread pool
 */
class Executor;
void perform(Executor * exec);
class Executor {
    enum class State {
        // Threadpool is fully operational, tasks could be added and get executed
        kRun,

        // Threadpool is on the way to be shutdown, no ned task could be added, but existing will be
        // completed as requested
        kStopping,

        // Threadppol is stopped
        kStopped
    };

public:
    /**
     * Main function that all pool threads are running. It polls internal task queue and execute tasks
     */
    friend void perform(Executor *executor);
    Executor(std::size_t low_watermark=2, std::size_t hight_watermark=7, std::size_t max_queue_size = 10, std::size_t idle_time = 1000);
    ~Executor();

    /**
     * Signal thread pool to stop, it will stop accepting new jobs and close threads just after each become
     * free. All enqueued jobs will be complete.
     *
     * In case if await flag is true, call won't return until all background jobs are done and all threads are stopped
     */
    void Stop(bool await = false);

    /**
     * Add function to be executed on the threadpool. Method returns true in case if task has been placed
     * onto execution queue, i.e scheduled for execution and false otherwise.
     *
     * That function doesn't wait for function result. Function could always be written in a way to notify caller about
     * execution finished by itself
     */
    template <typename F, typename... Types> bool Execute(F &&func, Types... args) {
        // Prepare "task"
        auto exec = std::bind(std::forward<F>(func), std::forward<Types>(args)...);

        std::cout << "Execute\n";

        std::unique_lock<std::mutex> lock(this->mutex);
        if (state != State::kRun) {
            return false;
        }

        if (tasks.size() < _max_queue_size)
        {
            tasks.push_back(exec);
        }
        else
        {
            return false;
        }

        if (_free_threads == 0 && _active_threads<_hight_watermark)
        {
//            std::thread th(perform, this);
//            _free_threads++;
//            th.detach();
        }
        empty_condition.notify_one();
        return true;
    }
private:
    // No copy/move/assign allowed
    Executor(const Executor &);            // = delete;
    Executor(Executor &&);                 // = delete;
    Executor &operator=(const Executor &); // = delete;
    Executor &operator=(Executor &&);      // = delete;

    /**
     * Mutex to protect state below from concurrent modification
     */
    std::mutex mutex;

    /**
     * Conditional variable to await new data in case of empty queue
     */
    std::condition_variable empty_condition;

    /**
     * Vector of actual threads that perorm execution
     */
    std::vector<std::thread> threads;

    /**
     * Task queue
     */
    std::deque<std::function<void()>> tasks;

    /**
     * Flag to stop bg threads
     */
    State state;
    std::size_t _low_watermark;
    std::size_t _hight_watermark;
    std::size_t _max_queue_size;
    std::size_t _idle_time;
    std::size_t _active_threads;
    std::size_t _free_threads;
};

} // namespace Concurrency
} // namespace Afina

#endif // AFINA_CONCURRENCY_EXECUTOR_H
