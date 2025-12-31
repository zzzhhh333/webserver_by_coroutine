#include "log.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

namespace nb {
namespace log {
Logger::Logger(const std::string &name)
    : running_(true)
    , name_(name)
{
    write_thread_ = std::thread(&Logger::write_loop, this);
}

void Logger::Log(Level level, const std::string &msg, std::vector<int> output_fd)
{
    std::unique_lock lock_(mtx_);
    if (!running_) {return;}
    msg_queue_.emplace(format_message(level, msg), output_fd);
    cond_v_.notify_one();
}

std::string Logger::format_message(Level level, const std:: string &msg) 
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;
    std::string level_msg;
    switch (level)
    {
    case Level::INFO :
        level_msg = "INFO";
        break;
    case Level::DEBUG :
        level_msg = "DEBUG";
        break;
    case Level::WARN :
        level_msg = "WARN";
        break;
    case Level::ERROR :
        level_msg = "ERROR";
        break;
    case Level::FATAL :
        level_msg = "FATAL";
        break;
    default:
        break;
    }

    std::stringstream ss;
    ss << "[" << std::put_time(localtime(&time_t), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
        << "[" << level_msg << "] "
        << msg;
    return ss.str();
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

            if (msg_data.output_fd.empty()) {
                std::cout << msg_data.msg <<std::endl;
            }

            for(auto &output : msg_data.output_fd) {
                if (output == 1) {
                    std::cout << msg_data.msg <<std::endl;
                }else {
                    std::string msg = msg_data.msg + '\n';
                    ::write(output, msg.c_str(), msg.size());
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