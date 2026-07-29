#include "../logs/yimingcode.h"
#include "bench.h"
#include "../extend/roll_by_time_sink.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

using namespace yimingcode;

// 同步日志测试函数
void sync_bench_thread_log(size_t thread_count, size_t msg_count, size_t msglen)
{
    static int num = 1;
    std::string logger_name = "sync_bench_logger" + std::to_string(num++);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "同步日志测试" << std::endl;
    std::cout << "线程数: " << thread_count << std::endl;
    std::cout << "日志条数: " << msg_count << std::endl;
    std::cout << "每条大小: " << msglen << " 字节" << std::endl;
    std::cout << "总数据量: " << (msglen * msg_count / 1024.0) << " KB" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建同步日志器
    std::unique_ptr<LoggerBuilder> lbp(new GlobalLoggerBuilder());
    lbp->buildLoggerName(logger_name);
    lbp->buildFormatter("[%c][%f:%l][%p]%m%n");
    lbp->buildSink<FileSink>("./logs/sync.log");
    lbp->buildLoggerType(LoggerType::LOGGER_SYNC);
    lbp->build();
    
    // 运行基准测试
    bench(logger_name, thread_count, msglen, msg_count);
}

// 异步日志测试函数
void async_bench_thread_log(size_t thread_count, size_t msg_count, size_t msglen)
{
    static int num = 1;
    std::string logger_name = "async_bench_logger" + std::to_string(num++);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "异步日志测试" << std::endl;
    std::cout << "线程数: " << thread_count << std::endl;
    std::cout << "日志条数: " << msg_count << std::endl;
    std::cout << "每条大小: " << msglen << " 字节" << std::endl;
    std::cout << "总数据量: " << (msglen * msg_count / 1024.0) << " KB" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建异步日志器
    std::unique_ptr<LoggerBuilder> lbp(new GlobalLoggerBuilder());
    lbp->buildLoggerName(logger_name);
    lbp->buildFormatter("[%c][%f:%l][%p]%m%n");
    lbp->buildSink<FileSink>("./logs/async.log");
    lbp->buildSink<StdoutSink>();
    lbp->buildLoggerType(LoggerType::LOGGER_ASYNC);
    lbp->buildEnableUnSafeAsync();
    lbp->build();
    
    // 运行基准测试
    bench(logger_name, thread_count, msglen, msg_count);
}

// 主测试函数（所有测试不超过5万条）
void bench_test() 
{
    system("mkdir -p ./logs");
    
    std::cout << "\n========== 日志系统性能测试 ==========" << std::endl;
    std::cout << "所有测试日志总数不超过 50000 条" << std::endl;
    
    // ========== 测试1: 异步日志 单线程 ==========
    async_bench_thread_log(1, 10000, 100);
    
    // ========== 测试2: 异步日志 多线程 ==========
    async_bench_thread_log(4, 50000, 100);
    
    // ========== 测试3: 异步日志 不同消息大小 ==========
    async_bench_thread_log(2, 10000, 256);
    async_bench_thread_log(2, 10000, 512);
    
    // ========== 测试4: 同步日志 单线程 ==========
    sync_bench_thread_log(1, 5000, 100);
    
    // ========== 测试5: 同步日志 多线程 ==========
    sync_bench_thread_log(4, 20000, 100);
    
    // ========== 测试6: 时间滚动异步日志 ==========
    std::cout << "\n========================================" << std::endl;
    std::cout << "时间滚动异步日志测试" << std::endl;
    std::cout << "线程数: 2" << std::endl;
    std::cout << "日志条数: 10000" << std::endl;
    std::cout << "========================================" << std::endl;
    
    static int num = 1;
    std::string logger_name = "roll_bench_logger" + std::to_string(num++);
    std::unique_ptr<LoggerBuilder> lbp(new GlobalLoggerBuilder());
    lbp->buildLoggerName(logger_name);
    lbp->buildFormatter("[%c][%f:%l][%p]%m%n");
    lbp->buildSink<RollByTimeSink>("./logs/roll-async.log", TimeGap::GAP_SECOND);
    lbp->buildLoggerType(LoggerType::LOGGER_ASYNC);
    lbp->buildEnableUnSafeAsync();
    lbp->build();
    bench(logger_name, 2, 100, 10000);
    
    // 等待异步日志完成
    std::cout << "\n等待异步日志写入完成..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "\n========== 所有测试完成 ==========" << std::endl;
}

int main(int argc, char *argv[])
{
    bench_test();
    return 0;
}