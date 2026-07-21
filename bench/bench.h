#ifndef __M_BENCH_H__
#define __M_BENCH_H__

#include "../logs/yimingcode.h"
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <iomanip>

namespace yimingcode {

void bench(const std::string &logger_name, size_t thread_num, size_t msglen, size_t msg_count)
{
    Logger::ptr lp = LoggerManager::getInstance().getLogger(logger_name);
    if (lp.get() == nullptr) {
        std::cerr << "Logger [" << logger_name << "] not found!" << std::endl;
        return;
    }
    
    std::string msg(msglen, 'A');
    size_t msg_count_per_thread = msg_count / thread_num;
    std::vector<double> cost_time(thread_num);
    std::vector<std::thread> threads;
    
    std::cout << "\n开始测试..." << std::endl;
    std::cout << "每线程条数: " << msg_count_per_thread << std::endl;
    
    auto total_start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < thread_num; i++) {
        threads.emplace_back([&, i](){
            auto start = std::chrono::high_resolution_clock::now();
            for(size_t j = 0; j < msg_count_per_thread; j++) {
                LOG_FATAL(lp, "%s", msg.c_str());
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto cost = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            cost_time[i] = cost.count();
            auto avg = msg_count_per_thread / cost_time[i];
            std::cout << "线程" << i << " 耗时: " << std::fixed << std::setprecision(3) 
                      << cost.count() << "s 平均: " << (size_t)avg << " 条/秒" << std::endl;
        });
    }
    
    for(auto &thr : threads) {
        thr.join();
    }
    
    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_cost = std::chrono::duration_cast<std::chrono::duration<double>>(total_end - total_start);
    
    double max_cost = 0;
    for (auto cost : cost_time) {
        max_cost = max_cost < cost ? cost : max_cost;
    }
    
    std::cout << "\n---------- 测试结果 ----------" << std::endl;
    std::cout << "总日志条数: " << msg_count << std::endl;
    std::cout << "总数据量: " << (msglen * msg_count / 1024.0) << " KB" << std::endl;
    std::cout << "总耗时: " << std::fixed << std::setprecision(3) << max_cost << " 秒" << std::endl;
    std::cout << "吞吐量: " << (size_t)(msg_count / max_cost) << " 条/秒" << std::endl;
    std::cout << "吞吐量: " << std::fixed << std::setprecision(2) 
              << (msglen * msg_count / max_cost / 1024.0 / 1024.0) << " MB/秒" << std::endl;
    std::cout << "-----------------------------" << std::endl;
}

} // namespace yimingcode

#endif // __M_BENCH_H__