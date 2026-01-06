#include "scheduler.h"
#include "log.h"
#include <unistd.h>

int main()
{
    auto co = nb::coroutine::Coroutine::ptr(new nb::coroutine::Coroutine([](){
        NB_LOG_INFO("Hello from coroutine!");
    }));

    nb::scheduler::Scheduler scheduler(1, "zh_nb");
    scheduler.start();
    scheduler.schedule([]() {
    for (int i = 0; i < 5; ++i) {
        NB_LOG_INFO("Coroutine iteration {}", i);
    }});
    // sleep(5);
    scheduler.schedule(co);
    std::vector<nb::scheduler::Scheduler::Task> more_tasks = {
        nb::scheduler::Scheduler::Task([]() {
            for (int i = 0; i < 2; ++i) {
                NB_LOG_INFO("More coroutine iteration {}", i);
                nb::coroutine::Coroutine::Yield();
            }}),
        nb::scheduler::Scheduler::Task([]() {
            for (int i = 0; i < 4; ++i) {
                NB_LOG_INFO("Additional coroutine iteration {}", i);
                nb::coroutine::Coroutine::Yield();
            }})
    };
    scheduler.schedule_more(more_tasks);
    sleep(10);
    scheduler.stop();
    NB_LOG_INFO("Hello from coroutine!");
    // sleep(2);
    return 0;
}