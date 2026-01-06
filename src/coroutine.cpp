#include "coroutine.h"
#include "log.h"
#include "util.h"
#include "scheduler.h"
namespace nb {
namespace coroutine {
static thread_local Coroutine* current_coroutine = nullptr;     //!  当前正在工作的协程
static std::atomic<uint64_t> s_fiber_id {0};                    //!  协程 ID 生成器
static std::atomic<uint64_t> s_fiber_count {0};                 //!  当前协程数量

Coroutine::Coroutine(std::function<void()> cb, size_t stack_size)
        : cb_(std::move(cb)) 
        , id_(++s_fiber_id)
        , state_(State::READY)
        , stack_size_(stack_size)
{
    s_fiber_count++;
    NB_LOG_INFO("Creating new coroutine, id: {}, total: {}", id_, s_fiber_count);
    stack_ = malloc(stack_size_);
    
    getcontext(&context_);
    context_.uc_stack.ss_sp = stack_;
    context_.uc_stack.ss_size = stack_size_;
    context_.uc_link = nullptr;
    makecontext(&context_, &Coroutine::CoroutineEntryPoint, 0);
}

Coroutine::Coroutine()
    : cb_(nullptr)
{   
    s_fiber_count++;
    id_ = ++s_fiber_id;
    state_ = State::RUNNING;
    current_coroutine = this;
    NB_LOG_INFO("Creating main coroutine, id: {}, total: {}", id_, s_fiber_count);
    getcontext(&context_);
}

uint64_t Coroutine::GetFiberId() {
    if (current_coroutine) {
        return current_coroutine->id_;
    }
    return 0;
}

void Coroutine::Resume() 
{
    // NB_LOG_INFO("state:{}",(int)state_);
    NB_ASSERT(state_ != State::FINISHED, "Cannot resume a finished coroutine");

    current_coroutine = this;
    state_ = State::RUNNING;
    swapcontext(&scheduler::Scheduler::GetMainContext()->context_, &context_);
    current_coroutine = nullptr;
}

void Coroutine::Yield() 
{
    NB_ASSERT(current_coroutine != nullptr, "Yield() called outside any coroutine");
    current_coroutine->state_ = State::READY;
    swapcontext(&current_coroutine->context_, &scheduler::Scheduler::GetMainContext()->context_);
}

Coroutine::~Coroutine() 
{
    s_fiber_count--;
    NB_LOG_INFO("Destroying coroutine, id: {}, total: {}", id_, s_fiber_count);
    if (stack_) {
        NB_ASSERT(state_ == State::FINISHED || state_ == State::EXCEPTION || state_ == State::READY,
              "Coroutine must nbe finished or in exception state before destruction");
        free(stack_);
    } else {
        NB_ASSERT(state_ == State::RUNNING,
              "Main coroutine must be in RUNNING state before destruction");
    }
}

Coroutine* Coroutine::GetThis() 
{
    return current_coroutine;
}

void Coroutine::CoroutineEntryPoint() 
{
    Coroutine* co = Coroutine::GetThis();
    NB_ASSERT(co->state_ == State::RUNNING, "Coroutine must be in RUNNING state at entry point");

    try 
    {
        if (co->cb_) {
            co->cb_();
        }
        co->state_ = State::FINISHED;
    } catch (const std::exception &e) {
        NB_LOG_ERROR("Coroutine exception: {}", e.what());
        co->state_ = State::EXCEPTION;
    } catch (...) {
        NB_LOG_ERROR("Coroutine unknown exception");
        co->state_ = State::EXCEPTION;
    }

    co->cb_ = nullptr;
    swapcontext(&co->context_, &scheduler::Scheduler::GetMainContext()->context_);
}

} // namespace coroutine
} // namespace nb


