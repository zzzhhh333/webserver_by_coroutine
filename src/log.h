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

/**
 * @brief 日志记录器类，支持多线程异步日志记录
 */
class Logger
{
public:
    /** 
     * @brief 日志级别枚举
     */
    enum class Level
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

private:
    // 日志数据结构体
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
    /**
     * @brief 构造函数，初始化日志记录器
     * @param name 日志记录器名称
     */
    explicit Logger(const std::string &name);

    ~Logger();

    /**
     * @brief 记录一条日志消息
     * @param level 日志级别
     * @param msg 日志消息内容
     * @param file 源文件名
     * @param line 行号
     * @param output_targets 输出目标，可以是文件名或 "stdout"
     * @param both_outputs 是否同时输出到 stdout 和指定目标
     */
    void Log(Level level, const std::string &msg,
             const char* file, int line,
             std::vector<std::string> output_targets = {"stdout"},
             bool both_outputs = false);    
    
    /** 
     * @brief 刷新日志，确保所有日志消息都已写出
     */
    void Flush();
    static Logger& GetInstance();

private:
    /**
     * @brief 格式化日志消息
     * @param level 日志级别
     * @param msg 日志消息内容
     * @param file 源文件名
     * @param line 行号
     * @return 格式化后的日志字符串
     */
    std::string format_message(Level level, const std::string &msg,
                                       const char* file, int line);
    
    /** 
     * @brief 日志写入线程的主循环函数
     */
    void write_loop();

private:
    std::string name_;                  // 日志记录器名称
    std::queue<log_data> msg_queue_;    // 日志消息队列
    std::thread write_thread_;          // 日志写入线程
    std::condition_variable cond_v_;    // 条件变量用于通知写线程
    std::mutex mtx_;                    // 互斥锁保护消息队列
    std::atomic_bool running_;          // 日志记录器是否正在运行
};


}
}


#endif