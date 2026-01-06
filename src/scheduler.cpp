#include "scheduler.h"
#include "log.h"
#include "util.h"
#include "coroutine.h"

namespace nb {
namespace scheduler {

static thread_local coroutine::Coroutine::ptr main_co;          //!  当前线程的主协程

Scheduler::Scheduler(int thread_num, const std::string name)
    : thread_num_(thread_num)
    , threads_pool_()
    , is_stop_(false)
    , name_(name)
{
    
}

Scheduler::~Scheduler()
{
    main_co = nullptr;
}

bool Scheduler::schedule_nonblock(Task task)
{
    bool need_tickle = task_queue_.empty();

    task_queue_.emplace_back(task);

    return need_tickle;
}

void Scheduler::start() {
    threads_pool_.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
        threads_pool_.emplace_back(&Scheduler::run, this);
    }
    NB_LOG_INFO("Started {} threads", thread_num_);
}

void Scheduler::stop()
{
    NB_LOG_INFO("Stopping scheduler");

    if (is_stop_) {
        return;
    }

    is_stop_ = true;
    
    std::vector<std::thread> cos;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        cos.swap(threads_pool_);
    }


    for (auto & thread : cos) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Scheduler::run()
{
    main_co = std::make_shared<coroutine::Coroutine>();

    coroutine::Coroutine::ptr idle_co_ = 
                        std::make_shared<coroutine::Coroutine>(std::bind(&Scheduler::idle, this));
    coroutine::Coroutine::ptr cb_co_;
    Task task;
    while(true)
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            for (auto it = task_queue_.begin(); it != task_queue_.end(); it++) {
                if (it->thread_id_ != -1 &&
                    it->thread_id_ != util::GetThreadId()) {
                    // 任务不属于当前线程，跳过
                    continue;
                }

                task = *it;
                task_queue_.erase(it);
                break;
            }
        }
        if (task.co_ || task.cb_) {
            if (task.co_) {
                task.co_->Resume();
                if (task.co_->getState() == coroutine::Coroutine::State::READY) {
                    std::lock_guard<std::mutex> lock(mtx_);
                    task_queue_.push_back(task);
                }
                task.co_ = nullptr;
            } else if (task.cb_) {
                cb_co_.reset(new coroutine::Coroutine(task.cb_));
                cb_co_->Resume();
                if (cb_co_->getState() == coroutine::Coroutine::State::READY) {
                    std::lock_guard<std::mutex> lock(mtx_);
                    task_queue_.push_back(Task(cb_co_));
                }
                task.cb_ = nullptr;
            } 
        } else {
            if (idle_co_->getState() == coroutine::Coroutine::State::FINISHED) {
                NB_LOG_INFO("Idle coroutine finished, exiting thread");
                break;
            }
            idle_co_->Resume();
        }
    }
}

void Scheduler::idle()
{
    while(!is_stop_ || !task_queue_.empty()) {
        // NB_LOG_INFO("enter idle !!!");
        coroutine::Coroutine::Yield();
    }
    
}

void Scheduler::tickle()
{
    NB_LOG_INFO("tickle");
}

coroutine::Coroutine::ptr& Scheduler::GetMainContext() {
    return main_co;
}

}
}