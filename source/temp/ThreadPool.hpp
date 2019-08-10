#ifndef H_THREADPOOL
#define H_THREADPOOL

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <stack>
#include <thread>

class ThreadPool
{
public:
    ThreadPool(size_t threads);
    virtual ~ThreadPool();

    template <class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    int PendingTaskCount();
    int RunningTaskCount();
    void WaitForThreads();

private:
    std::vector<std::thread>          m_vWorkers;
    std::stack<std::function<void()>> m_qTasks;
    std::mutex                        m_mutex;
    std::condition_variable           m_condition;

    std::mutex m_syncMutex;
    std::condition_variable m_syncCondition;

    bool                              m_bStop;
    int                               m_iRunningTasks = 0;
};

// template definition
template <class F, class... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_bStop)
        {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        m_qTasks.emplace([task]() { (*task)(); });
    }
    m_condition.notify_one();
    return res;
}

#endif
