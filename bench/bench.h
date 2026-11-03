#ifndef __M_BENCH_H__
#define __M_BENCH_H__

#include "../logs/yimingcode.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace yimingcode {

struct BenchResult {
    double producer_seconds;
    double drain_seconds;
    size_t written_lines;
    size_t written_bytes;
};

inline size_t countLines(const std::string& path)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open benchmark output: " + path);
    size_t lines = 0;
    char ch;
    while (input.get(ch)) if (ch == '\n') ++lines;
    if (!input.eof()) throw std::runtime_error("Cannot read benchmark output: " + path);
    return lines;
}

inline size_t fileSize(const std::string& path)
{
    std::ifstream input(path.c_str(), std::ios::binary | std::ios::ate);
    if (!input) throw std::runtime_error("Cannot inspect benchmark output: " + path);
    const std::streamoff size = input.tellg();
    if (size < 0) throw std::runtime_error("Cannot determine benchmark output size: " + path);
    return static_cast<size_t>(size);
}

inline BenchResult bench(const std::string& logger_name, bool asynchronous,
                         size_t thread_num, size_t msglen, size_t msg_count,
                         const std::string& output_path)
{
    if (thread_num == 0 || msg_count == 0 || msg_count % thread_num != 0)
        throw std::invalid_argument("message count must be positive and divisible by thread count");

    std::remove(output_path.c_str());
    const size_t per_thread = msg_count / thread_num;
    const std::string msg(msglen, 'A');
    auto logger = LoggerManager::getInstance().getLogger(logger_name);
    if (!logger) throw std::runtime_error("Logger not found: " + logger_name);

    typedef std::chrono::steady_clock Clock;
    const Clock::time_point start = Clock::now();
    std::vector<std::thread> producers;
    producers.reserve(thread_num);
    for (size_t i = 0; i < thread_num; ++i) {
        producers.emplace_back([logger, &msg, per_thread]() {
            for (size_t j = 0; j < per_thread; ++j)
                LOG_FATAL(logger, "%s", msg.c_str());
        });
    }
    for (auto& producer : producers) producer.join();
    const Clock::time_point producers_done = Clock::now();

    // Destroying the last async logger reference stops and joins its worker,
    // which drains all accepted buffers before returning.
    logger.reset();
    const Clock::time_point drained = Clock::now();

    BenchResult result;
    result.producer_seconds = std::chrono::duration<double>(producers_done - start).count();
    result.drain_seconds = asynchronous
        ? std::chrono::duration<double>(drained - producers_done).count()
        : 0.0;
    result.written_lines = countLines(output_path);
    result.written_bytes = fileSize(output_path);
    if (result.written_lines != msg_count)
        throw std::runtime_error("Expected " + std::to_string(msg_count) +
                                 " lines, found " + std::to_string(result.written_lines));
    return result;
}

inline void printResult(const std::string& label, const BenchResult& result,
                        size_t messages, bool asynchronous)
{
    const double total = result.producer_seconds + result.drain_seconds;
    std::cout << "\n" << label << "\n"
              << "  producer time: " << std::fixed << std::setprecision(3)
              << result.producer_seconds << " s (" << std::setprecision(0)
              << messages / result.producer_seconds << " messages/s)\n";
    if (asynchronous)
        std::cout << "  drain time: " << std::fixed << std::setprecision(3)
                  << result.drain_seconds << " s\n";
    std::cout << "  end-to-end time: " << std::fixed << std::setprecision(3)
              << total << " s (" << std::setprecision(0) << messages / total
              << " messages/s)\n"
              << "  verified output: " << result.written_lines << " lines, "
              << result.written_bytes << " bytes\n";
}

} // namespace yimingcode

#endif
