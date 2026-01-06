#ifndef NB_UTIL_H
#define NB_UTIL_H

#include <sys/syscall.h>
#include <unistd.h>

// util.h
#define NB_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            nb::log::Logger::GetInstance().Log( \
                nb::log::Logger::Level::ERROR, \
                "Assertion failed: " #cond ", reason: " msg, \
                __FILE__, __LINE__, {}, true); \
            nb::log::Logger::GetInstance().Flush(); /* 👈 关键：等待日志写出 */ \
            std::abort(); /* 比 assert(false) 更可控 */ \
        } \
    } while(0)

namespace nb {
namespace util {

inline uint64_t GetThreadId()
{
    return static_cast<uint64_t>(::syscall(SYS_gettid));
}


}
}
#endif // NB_UTIL_H
