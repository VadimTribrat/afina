#include <afina/concurrency/Executor.h>

namespace Afina {
namespace Concurrency {
    Executor::Executor(std::size_t low_watermark, std::size_t hight_watermark, std::size_t max_queue_size, std::size_t idle_time):
    _low_watermark(low_watermark), _hight_watermark(hight_watermark), _max_queue_size(max_queue_size), _idle_time(idle_time), 
    _active_threads(0), _free_threads(_hight_watermark), state(Executor::State::kRun)
    {
        for (int i = 0; i < _hight_watermark; i++)
        {
            std::thread th(perform, this);
            th.detach();
        }
    }
    Executor::~Executor()
    {
        std::cout << "Number of threads: " << _free_threads + _active_threads << " Tasks: " << tasks.size() << "\n";
    }
    void Executor::Stop(bool await)
    {
        std::unique_lock<std::mutex> lock(mutex);
        state = State::kStopping;
        empty_condition.notify_all();
        if (await) {
            while (_active_threads + _free_threads > 0) {
                empty_condition.wait(lock);
            }
        }
    }
    void perform(Executor * exec)
    {
        std::function<void()> func;
        auto last_task_time = std::chrono::system_clock::now();
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(exec->mutex);
                while (exec->tasks.empty())
                {
                    exec->empty_condition.wait_for(lock, exec->_idle_time);
                    if (exec->state != Executor::State::kRun && exec->tasks.empty())
                    {
                        std::cout << "kStopped\n";
                        exec->state = Executor::State::kStopped;
                        exec->_free_threads--;
                        exec->empty_condition.notify_all();
                        return;
                    }
                    auto now = std::chrono::system_clock::now();
                    if (now - last_task_time >exec->_idle_time && exec->_free_threads>exec->_low_watermark)
                    {
                        exec->_free_threads--;
                        exec->empty_condition.notify_one();
                        std::cout << "- thread\n";
                        return;
                    }
                }
                exec->_free_threads--;
                exec->_active_threads++;
                func = exec->tasks.front();
                exec->tasks.pop_front();
    //            exec->empty_condition.notify_one();
            }
            exec->empty_condition.notify_one();
            std::this_thread::sleep_for(std::chrono::seconds(10));
            func();
            std::unique_lock<std::mutex> lock(exec->mutex);
            exec->_free_threads++;
            exec->_active_threads--;
        }
    }
}
} // namespace Afina
