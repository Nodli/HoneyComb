#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threads)
    : m_bStop(false)
{
    for (size_t i = 0; i < threads; ++i)
        m_vWorkers.emplace_back([this] {
            while (true)
            {
                std::function<void()> task;

                {
                    // waits until the thread must stop or a task is added to the stack
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_condition.wait(lock, [this] { return m_bStop || !m_qTasks.empty(); });

                    // when asked via the m_bStop, stops the thread only when the task stack is emptied
                    if (m_bStop && m_qTasks.empty())
                    {
                        return;
                    }

                    // recover the next pending task
                    task = std::move(m_qTasks.top());
                    m_qTasks.pop();
                    ++m_iRunningTasks;
                }

                // execute task and report termination to the threadpool
                task();

                std::unique_lock<std::mutex> lock(m_mutex);
                --m_iRunningTasks;
                m_syncCondition.notify_all();
            }
        });
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_bStop = true;
    }

    // notifies all waiting threads that the wait condition may have changed
    m_condition.notify_all();

    // wait for the termination of all threads
    for (std::thread& worker : m_vWorkers)
    {
        worker.join();
    }
}

int ThreadPool::PendingTaskCount()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_qTasks.size();
}

int ThreadPool::RunningTaskCount()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_iRunningTasks;
}

void ThreadPool::WaitForThreads(){

    auto syncFunction = [this](){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_qTasks.empty() && (m_iRunningTasks == 0);
    };

    // waits until the task queue is empty and all thread are done
    std::unique_lock<std::mutex> lock(m_syncMutex);
    m_syncCondition.wait(lock, syncFunction);
}
