#include "coroutine.h"
#include "log.h"
#include "util.h"
#include "scheduler.h"
namespace nb {
namespace coroutine {
static thread_local Coroutine* current_coroutine = nullptr;     //!  当前正在工作的协程

Coroutine::Coroutine(std::function<void()> cb)
        : cb_(std::move(cb)) 
{
    stack_ = new char[stack_size_];
    
    getcontext(&context_);
    context_.uc_stack.ss_sp = stack_;
    context_.uc_stack.ss_size = stack_size_;
    context_.uc_link = &scheduler::Scheduler::GetMainContext();
    makecontext(&context_, (void (*)()) &Coroutine::CoroutineEntryPoint, 1, this);
}

Coroutine::Coroutine()
    : cb_(nullptr)
{
    stack_ = new char[stack_size_];
    
    getcontext(&context_);
}

void Coroutine::Resume() 
{
    // NB_LOG_INFO("state:{}",(int)state_);
    NB_ASSERT(state_ != State::FINISHED, "Cannot resume a finished coroutine");

    current_coroutine = this;
    state_ = State::RUNNING;
    swapcontext(&scheduler::Scheduler::GetMainContext(), &context_);
    current_coroutine = nullptr;
}

void Coroutine::Yield() 
{
    NB_ASSERT(current_coroutine != nullptr, "Yield() called outside any coroutine");
    current_coroutine->state_ = State::READY;
    swapcontext(&current_coroutine->context_, &scheduler::Scheduler::GetMainContext());
}

Coroutine::~Coroutine() 
{
    NB_ASSERT(state_ == State::FINISHED || state_ == State::EXCEPTION || state_ == State::READY,
              "Coroutine must be finished or in exception state before destruction");
    delete[] stack_;
}

void Coroutine::CoroutineEntryPoint(Coroutine* co) 
{
    NB_ASSERT(co->state_ == State::RUNNING, "Coroutine must be in RUNNING state at entry point");

    try 
    {
        if (co->cb_) {
            co->cb_();
        }
        NB_LOG_INFO("Coroutine finished");
        co->state_ = State::FINISHED;
    } catch (const std::exception &e) {
        NB_LOG_ERROR("Coroutine exception: {}", e.what());
        co->state_ = State::EXCEPTION;
    } catch (...) {
        NB_LOG_ERROR("Coroutine unknown exception");
        co->state_ = State::EXCEPTION;
    }

    NB_LOG_INFO("Coroutine exiting, switching back to main context");
}

} // namespace coroutine
} // namespace nb


