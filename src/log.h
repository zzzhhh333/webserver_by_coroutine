#ifndef NB_LOG_H
#define NB_LOG_H

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>
#include <fmt/format.h>

#define NB_LOG_IMPL(level, output_targets, msg, both_outputs, ...) \
    nb::log::Logger::GetInstance().Log( \
        nb::log::Logger::Level::level, \
        fmt::format(msg, ##__VA_ARGS__), \
        __FILE__, __LINE__, \
        output_targets, \
        both_outputs)

// 默认输出到 stdout
#define NB_LOG_DEBUG(msg, ...)   NB_LOG_IMPL(DEBUG, {"stdout"}, msg, false, ##__VA_ARGS__)
#define NB_LOG_INFO(msg, ...)    NB_LOG_IMPL(INFO,  {"stdout"}, msg, false, ##__VA_ARGS__)
#define NB_LOG_WARN(msg, ...)    NB_LOG_IMPL(WARN,  {"stdout"}, msg, false, ##__VA_ARGS__)
#define NB_LOG_ERROR(msg, ...)   NB_LOG_IMPL(ERROR, {"stdout"}, msg, false, ##__VA_ARGS__)
#define NB_LOG_FATAL(msg, ...)   NB_LOG_IMPL(FATAL, {"stdout"}, msg, false, ##__VA_ARGS__)
// 指定输出目标（文件名或 "stdout"/"stderr"）
#define NB_LOG_DEBUG_FILE(file_name, msg, ...)   NB_LOG_IMPL(DEBUG, {file_name}, msg, true, ##__VA_ARGS__)
#define NB_LOG_INFO_FILE(file_name, msg, ...)    NB_LOG_IMPL(INFO,  {file_name}, msg, true, ##__VA_ARGS__)
#define NB_LOG_WARN_FILE(file_name, msg, ...)    NB_LOG_IMPL(WARN,  {file_name}, msg, true, ##__VA_ARGS__)
#define NB_LOG_ERROR_FILE(file_name, msg, ...)   NB_LOG_IMPL(ERROR, {file_name}, msg, true, ##__VA_ARGS__)
#define NB_LOG_FATAL_FILE(file_name, msg, ...)   NB_LOG_IMPL(FATAL, {file_name}, msg, true, ##__VA_ARGS__)
// 仅输出到指定文件
#define NB_LOG_DEBUG_ONLY_FILE(file_name, msg, ...) NB_LOG_IMPL(DEBUG, {file_name}, msg, false, ##__VA_ARGS__)
#define NB_LOG_INFO_ONLY_FILE(file_name, msg, ...)  NB_LOG_IMPL(INFO,  {file_name}, msg, false, ##__VA_ARGS__)
#define NB_LOG_WARN_ONLY_FILE(file_name, msg, ...)  NB_LOG_IMPL(WARN,  {file_name}, msg, false, ##__VA_ARGS__)
#define NB_LOG_ERROR_ONLY_FILE(file_name, msg, ...) NB_LOG_IMPL(ERROR, {file_name}, msg, false, ##__VA_ARGS__) 
#define NB_LOG_FATAL_ONLY_FILE(file_name, msg, ...) NB_LOG_IMPL(FATAL, {file_name}, msg, false, ##__VA_ARGS__)
namespace nb {
namespace log {
class Logger
{
public:
    enum class Level
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

private:
    struct log_data
    {
        log_data(const std::string &m, const std::vector<std::string> &output)
            : msg(m)
            , output(output)
        {}
        std::string msg;
        std::vector<std::string> output;
    };

public:
    explicit Logger(const std::string &name);
    ~Logger();
    void Log(Level level, const std::string &msg,
             const char* file, int line,
             std::vector<std::string> output_targets = {"stdout"},
             bool both_outputs = false);    
    static Logger& GetInstance();

private:
    std::string format_message(Level level, const std::string &msg,
                                       const char* file, int line);
    void write_loop();

private:
    std::string name_;
    std::queue<log_data> msg_queue_;
    std::thread write_thread_;
    std::condition_variable cond_v_;
    std::mutex mtx_;
    std::atomic_bool running_;
};


}
}


#endif