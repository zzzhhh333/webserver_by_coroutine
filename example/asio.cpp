#include "coroutine.h"
#include "log.h"


int main() {
    nb::coroutine::Coroutine::ptr co = std::make_shared<nb::coroutine::Coroutine>([]() {
        for (int i = 0; i < 5; ++i) {
            NB_LOG_INFO("Coroutine iteration {}", i);
            nb::coroutine::Coroutine::Yield();
            NB_LOG_INFO("Coroutine iteration again {}", i);
            nb::coroutine::Coroutine::Yield();
        }
    });

    for (int i = 0; i < 5; ++i) {
        NB_LOG_INFO("Main iteration {}", i);
        co->Resume();
    }

    while(true) {
        NB_LOG_INFO("Main final loop");
        co->Resume();
        if (co->getState() == nb::coroutine::Coroutine::State::FINISHED) {
            break;
        }
    }

    return 0;
}