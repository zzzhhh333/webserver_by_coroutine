#include "log.h"
#include <iostream>
#include <fstream>
#include "util.h"

namespace nb {
namespace log {
Logger::Logger(const std::string &name)
    : running_(true)
    , name_(name)
{
    write_thread_ = std::thread(&Logger::write_loop, this);
}

Logger::~Logger()
{
    {
        std::unique_lock<std::mutex> lock(mtx_);
        running_ = false;
    }
    cond_v_.notify_all(); // 唤醒写线程

    if (write_thread_.joinable()) {
        write_thread_.join();
    }
}

void Logger::Log(Level level, const std::string &msg,
                 const char* file, int line,
                 std::vector<std::string> output_targets,
                 bool both_outputs)
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (!running_) return;
    if (both_outputs) {
        output_targets.push_back("stdout");
    }
    msg_queue_.emplace(format_message(level, msg, file, line), output_targets);
    cond_v_.notify_one();
}

void Logger::Flush() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!running_) return;

    // 唤醒写线程，并等待队列清空
    cond_v_.notify_all();
    // 等待直到队列为空（或超时）
    constexpr int max_wait_ms = 2000;
    auto start = std::chrono::steady_clock::now();
    while (!msg_queue_.empty()) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > max_wait_ms) {
            break; // 超时，避免卡死
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        lock.lock();
    }
}

Logger& Logger::GetInstance()
{
    static Logger instance("default_logger");
    return instance;
}

std::string Logger::format_message(Level level, const std::string &msg,
                                   const char* file, int line)
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    // 获取进程 ID 和线程 ID
    pid_t pid = getpid();
    pid_t tid = util::GetThreadId(); // ← 获取线程 ID

    // 提取文件名
    const char* filename = strrchr(file, '/');
    if (filename == nullptr) {
        filename = strrchr(file, '\\');
    }
    if (filename != nullptr) {
        file = filename + 1;
    }

    std::string level_str;
    switch (level) {
        case Level::DEBUG: level_str = "DEBUG"; break;
        case Level::INFO:  level_str = "INFO";  break;
        case Level::WARN:  level_str = "WARN";  break;
        case Level::ERROR: level_str = "ERROR"; break;
        case Level::FATAL: level_str = "FATAL"; break;
        default:           level_str = "UNKNOWN";
    }

    std::ostringstream oss;
    oss << "[" << std::put_time(localtime(&time_t), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
        << "[PID:" << pid << "] "
        << "[TID:" << tid << "] "   // ← 关键新增
        << "[" << level_str << "] "
        << "[" << file << ":" << line << "] "
        << msg;

    return oss.str();
}

void Logger::write_loop()
{
    while (true) {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_v_.wait(lock, [this] { return !msg_queue_.empty() || !running_; });

        while (!msg_queue_.empty()) {
            auto msg_data = std::move(msg_queue_.front());
            msg_queue_.pop();
            lock.unlock();

            if (msg_data.output.empty()) {
                std::cout << msg_data.msg <<std::endl;
            }

            for(auto &output : msg_data.output) {
                if (output == "stdout") {
                    std::cout << msg_data.msg <<std::endl;
                }else {
                    std::ofstream ofs(output, std::ios::app);
                    if (ofs.is_open()) {
                        ofs << msg_data.msg << std::endl;
                        ofs.close();
                    } else {
                        std::cerr << "Failed to open log file: " << output << std::endl;
                    }
                }
            }
            lock.lock();
        }

        if (!running_ && msg_queue_.empty()) {
            break;
        }
    }
}

}
}