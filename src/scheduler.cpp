#include "scheduler.h"
#include "log.h"
#include "util.h"
#include "coroutine.h"
#include <iostream>

namespace nb {
namespace scheduler {
static thread_local ucontext_t main_context;

Scheduler::Scheduler(int thread_num, const std::string name)
    : thread_num_(thread_num)
    , threads_pool_()
    , is_stop_(false)
    , name_(name)
{
    
}

Scheduler::~Scheduler()
{
    stop();
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
    if (is_stop_) {
        return;
    }

    is_stop_ = true;
    
    for (auto & thread : threads_pool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Scheduler::run()
{
    getcontext(&main_context);

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
                NB_LOG_DEBUG("1");
                break;
            }
        }
        if (task.co_ || task.cb_) {
            if (task.co_) {
                task.co_->Resume();
                NB_LOG_INFO("Coroutine resumed");
                if (task.co_->getState() == coroutine::Coroutine::State::READY) {
                    std::lock_guard<std::mutex> lock(mtx_);
                    task_queue_.push_back(task);
                    NB_LOG_INFO("Coroutine yielded, rescheduled");
                }
                task.co_ = nullptr;
            } else if (task.cb_) {
                NB_LOG_INFO("Starting callback task in new coroutine");
                cb_co_.reset(new coroutine::Coroutine(task.cb_));
                NB_LOG_INFO("Callback coroutine created, resuming");
                cb_co_->Resume();
                task.cb_ = nullptr;
            } 
        } else {
            idle_co_->Resume();
            if (is_stop_) {
                break;
            }
        }
        
    }
}

void Scheduler::idle()
{
    while(!is_stop_) {
        //NB_LOG_INFO("enter idle !!!");
        coroutine::Coroutine::Yield();
    }
    
}

void Scheduler::tickle()
{
    NB_LOG_INFO("tickle");
}

ucontext_t& Scheduler::GetMainContext() {
    return main_context;
}

}
}