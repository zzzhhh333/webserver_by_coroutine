cmake_minimum_required(VERSION 3.14)  # 升级到 3.14 以支持 FetchContent
project(WebServerByCoroutine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 自动获取 fmt（如果未找到）===
include(FetchContent)

# 尝试查找系统已安装的 fmt
find_package(fmt QUIET)

if(NOT fmt_FOUND)
    message(STATUS "fmt not found. Fetching from GitHub...")
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://ghfast.top/github.com/fmtlib/fmt.git
        GIT_TAG        10.2.1  # 使用稳定版本（可选最新）
    )
    FetchContent_MakeAvailable(fmt)
endif()

# 主程序源码
file(GLOB SOURCES "src/*.cpp")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 创建主库
add_library(webserver_by_coroutine ${SOURCES})

target_include_directories(webserver_by_coroutine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接 fmt（现在 fmt::fmt 一定可用）
target_link_libraries(webserver_by_coroutine PUBLIC fmt::fmt)

# === 测试部分 ===
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "example/*.cpp")

foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE webserver_by_coroutine)
endforeach()