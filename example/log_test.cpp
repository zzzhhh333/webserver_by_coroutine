#include "log.h"

int main()
{
    NB_LOG_INFO("This is an info message: {},hello {}", 42, "world");
    NB_LOG_ERROR_FILE("hello.txt", "This is an error message to stderr: {}", "error occurred");
    NB_LOG_DEBUG_ONLY_FILE("debug.log", "This is a debug message to debug.log: {}", 3.14);
    return 0;
}