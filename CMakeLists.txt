cmake_minimum_required(VERSION 3.10)
project(WebServerByCoroutine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 fmt 库（必须！）
find_package(fmt REQUIRED)

# 主程序源码（用于构建库）
file(GLOB SOURCES "src/*.cpp")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 创建主库
add_library(webserver_by_coroutine ${SOURCES})

# 设置库的 include 目录（让使用者能 include "nb_log.h" 等）
target_include_directories(webserver_by_coroutine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接 fmt 到你的库（因为 nb_log.cpp 用了 fmt）
target_link_libraries(webserver_by_coroutine PUBLIC fmt::fmt)

# === 测试部分 ===
# 自动查找 example/ 下所有以 test 结尾或包含 test 的 .cpp 文件（可自定义）
# 更推荐显式列出，但这里用 GLOB 方便演示多测试
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "example/*.cpp")

# 为每个测试源文件生成一个可执行文件
foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_src} NAME_WE)
    
    add_executable(${test_name} ${test_src})
    
    # 链接主库（包含 Logger 实现）和 fmt（虽然已通过库传递，但显式更安全）
    target_link_libraries(${test_name} PRIVATE webserver_by_coroutine)
endforeach()