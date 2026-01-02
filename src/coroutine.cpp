#include "coroutine.h"
#include "log.h"
#include "util.h"
namespace nb {
namespace coroutine {
static thread_local ucontext_t main_context_;
static thread_local Coroutine* current_coroutine_ = nullptr;

Coroutine::Coroutine(std::function<void()> cb)
        : cb_(std::move(cb)) 
{
    stack_ = new char[stack_size_];

    static thread_local bool main_inited = false;
    if (!main_inited) {
        getcontext(&main_context_);
        main_inited = true;
    }

    getcontext(&context_);
    context_.uc_stack.ss_sp = stack_;
    context_.uc_stack.ss_size = stack_size_;
    context_.uc_link = &main_context_;
    makecontext(&context_, (void (*)()) &Coroutine::CoroutineEntryPoint, 1, this);
}

void Coroutine::Resume() 
{
    NB_ASSERT(state_ != State::FINISHED, "Cannot resume a finished coroutine");
    swapcontext(&main_context_, &context_);
}

void Coroutine::Yield() 
{
    NB_ASSERT(current_coroutine_ != nullptr, "Yield() called outside any coroutine");
    current_coroutine_->state_ = State::READY;
    swapcontext(&current_coroutine_->context_, &main_context_);
}

Coroutine::~Coroutine() 
{
    NB_ASSERT(state_ == State::FINISHED || state_ == State::EXCEPTION,
              "Coroutine must be finished or in exception state before destruction");
    delete[] stack_;
}

void Coroutine::CoroutineEntryPoint(Coroutine* co) 
{
    Coroutine::ptr co_ptr = co->shared_from_this();
    Coroutine* co_ = co_ptr.get();

    NB_ASSERT(co_->state_ == State::READY, "Coroutine must be in READY state at entry point");

    co_->state_ = State::RUNNING;
    current_coroutine_ = co_;

    try 
    {
        if (co_->cb_) {
            co_->cb_();
        }
        co_->state_ = State::FINISHED;
        current_coroutine_ = nullptr;
    } catch (const std::exception &e) {
        NB_LOG_ERROR("Coroutine exception: {}", e.what());
        co_->state_ = State::EXCEPTION;
    } catch (...) {
        NB_LOG_ERROR("Coroutine unknown exception");
        co_->state_ = State::EXCEPTION;
    }

    current_coroutine_ = nullptr;
}

} // namespace coroutine
} // namespace nb


