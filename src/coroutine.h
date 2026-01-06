#ifndef NB_COROUTINE_H
#define NB_COROUTINE_H

#include <ucontext.h>
#include <functional>
#include <memory>

namespace nb {
namespace coroutine {

class Coroutine : public std::enable_shared_from_this<Coroutine> {
public:
    enum class State {
        READY = 0,
        RUNNING,
        FINISHED,
        EXCEPTION
    };

public:
    using ptr = std::shared_ptr<Coroutine>;
    /** 
     * @brief 构造函数，创建一个协程并初始化其上下文
     * @param cb 协程执行的函数
     */
    explicit Coroutine(std::function<void()> cb, size_t stack_size = 1024 * 1024);

    explicit Coroutine();
    
    /** 
     * @brief 析构函数，释放协程的栈空间
     */
    ~Coroutine();

    /**
     * @brief 恢复协程的执行
     * @return void
     * 
     */
    void Resume();

    State getState() const { return state_; }

    /**
     * @brief 暂停当前协程的执行，切换回主协程
     * @return void
     * 
     */
    static void Yield();

    /** 
     * @brief 获取当前协程对象
     * @return 当前协程指针
     */
    static Coroutine* GetThis();

    /** 
     * @brief 获取当前协程的 ID
     * @return 协程 ID
     */
    static uint64_t GetFiberId();
private:
    //! 协程的入口函数
    static void CoroutineEntryPoint(); 
private:
    ucontext_t context_;                        // 协程上下文
    const int stack_size_ = 1024 * 1024;        // 1 MB
    void *stack_ = nullptr;                     // 协程栈空间
    std::function<void()> cb_;                  // 协程执行的函数
    State state_ = State::READY;                // 协程状态
    int id_;                                    // 协程 ID
};

}
}

#endif // NB_COROUTINE_H