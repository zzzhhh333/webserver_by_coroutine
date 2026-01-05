#include "scheduler.h"
#include "log.h"
#include <unistd.h>
#include <signal.h>


void crash_handler(int sig) {
    NB_LOG_FATAL("Caught signal: {}", sig);
    nb::log::Logger::GetInstance().Flush();
    exit(1);
}

int main()
{
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    auto co = nb::coroutine::Coroutine::ptr(new nb::coroutine::Coroutine([](){
        NB_LOG_INFO("Hello from coroutine!");
    }));

    nb::scheduler::Scheduler scheduler(1, "zh_nb");
    scheduler.start();
    // scheduler.schedule([]() {
    // for (int i = 0; i < 5; ++i) {
    //     NB_LOG_INFO("Coroutine iteration {}", i);
    // }});
    // sleep(5);
    scheduler.schedule(co);
    sleep(5);
    scheduler.stop();
    NB_LOG_INFO("Hello from coroutine!");
    return 0;
}