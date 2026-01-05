#ifndef NB_SCHEDULER_H
#define NB_SCHEDULER_H

#include "coroutine.h"

#include <list>
#include <mutex>
#include <thread>

namespace nb {
namespace scheduler {

/**
 * @brief 协程调度器类，负责管理和调度协程的执行
 */
class Scheduler 
{
public:
    using ptr = std::shared_ptr<Scheduler>;

public:
    /**
     * @brief 任务结构体，可以是协程或普通函数
     */
    struct Task 
    {
        Task(nb::coroutine::Coroutine::ptr co): co_(co), cb_(nullptr) {}
        Task(std::function<void()> cb): co_(nullptr), cb_(cb) {}
        Task(): co_(nullptr), cb_(nullptr) {}

        nb::coroutine::Coroutine::ptr co_;
        std::function<void()> cb_;
        int thread_id_ = -1; // 任务指定运行的线程ID，-1表示不指定
    };

public:
    /**
     * @brief 构造函数，创建调度器并初始化线程数和名称
     * @param thread_num 线程数量
     * @param name 调度器名称
     */
    Scheduler(int thread_num, const std::string name);

    /**
     * @brief 析构函数，停止调度器并释放资源
     */
    virtual ~Scheduler();

    /**
     * @brief 调度一个任务，可以是协程或普通函数
     * @tparam T 任务类型，可以是 Coroutine::ptr 或 std::function<void()>
     * @param t 任务对象
     */
    template<typename T>
    void schedule(T t)
    {
        bool need_tickle = false;
        Task task(t);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            need_tickle = schedule_nonblock(task);
        }
        if (need_tickle) {
            tickle();
        }
    }

    /**
     * @brief 调度多个任务，可以是协程或普通函数
     * @tparam Container 任务容器类型，支持 STL 容器
     * @param tasks 任务容器
     */
    template<typename Container>
    void schedule_more(const Container& tasks) 
    {
        bool need_tickle = false;
        {
            for (typename Container::const_iterator it = tasks.begin();
                 it != tasks.end(); ++it) 
            {
                need_tickle = schedule_nonblock(*it);
            }
        }
        if (need_tickle) {
            tickle();
        }
    }

    /**
     * @brief 启动调度器，创建线程并开始调度任务
     */
    void start();

    /**
     * @brief 停止调度器，等待所有线程结束
     */
    void stop();

    static ucontext_t& GetMainContext();

private:
    /**
     * @brief 非阻塞方式调度一个任务
     * @param task 任务对象
     * @return 如果任务队列之前为空，返回 true，否则返回 false
     */
    bool schedule_nonblock(Task task);

    /**
     * @brief 唤醒调度器的一个线程，通知有新任务到来
     */
    virtual void tickle();

    /**
     * @brief 调度器的主循环函数，在线程中运行
     */
    void run();

    /**
     * @brief 空闲协程函数，当没有任务可执行时运行
     */
    virtual void idle();

private:
    std::list<Task> task_queue_;                // 任务队列
    std::vector<std::thread> threads_pool_;     // 线程池
    std::mutex mtx_;                            // 互斥锁保护任务队列
    int thread_num_;                            // 线程数量
    bool is_stop_;                              // 调度器是否停止
    const std::string name_;                    // 调度器名称
};

}
}
















#endif