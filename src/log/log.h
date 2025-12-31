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
        std::string msg;
        std::vector<int> output_fd;
    };

public:
    explicit Logger(const std::string &name);
    void Log(Level level, const std::string &msg, std::vector<int> output_fd = {1});

private:
    

    std::string format_message(Level level, const std:: string& msg);
    void write_loop();

private:
    std::string name_;
    std::queue<const log_data&> msg_queue_;
    std::thread write_thread_;
    std::condition_variable cond_v_;
    std::mutex mtx_;
    std::atomic_bool running_;
};


}
}


#endif