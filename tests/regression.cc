#include "logs/yimingcode.h"
#include <atomic>
#include <fstream>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

class CountingSink : public yimingcode::LogSink {
public:
    std::atomic<size_t> lines{0};
    void log(const char* data, size_t len) override {
        for (size_t i = 0; i < len; ++i) if (data[i] == '\n') ++lines;
    }
};

int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "smoke";
    if (mode == "stop-full") {
        std::promise<void> in_callback, release_callback;
        auto release = release_callback.get_future().share();
        std::atomic<int> batches{0};
        yimingcode::ASyncLogLooper looper([&](yimingcode::Buffer&) {
            if (++batches == 1) { in_callback.set_value(); release.wait(); }
        }, yimingcode::AsyncType::ASYNC_SAFE);
        looper.push("a", 1);
        in_callback.get_future().wait();
        std::string fill(DEFAULT_BUFFER_SIZE, 'X');
        looper.push(fill.data(), fill.size());
        auto producer = std::async(std::launch::async, [&] { looper.push("b", 1); });
        auto stopper = std::async(std::launch::async, [&] { looper.stop(); });
        bool woke = producer.wait_for(std::chrono::seconds(2)) == std::future_status::ready;
        release_callback.set_value();
        producer.get(); stopper.get();
        std::cout << "stop-full producer_returned=" << woke << " batches_drained=" << batches << '\n';
        return woke && batches == 2 ? 0 : 1;
    }
    if (mode == "smoke-sync" || mode == "smoke-async") {
        auto sink = std::make_shared<CountingSink>();
        auto fmt = std::make_shared<yimingcode::Formatter>("%m%n");
        std::vector<yimingcode::LogSink::ptr> sinks{sink};
        yimingcode::Logger::ptr logger;
        if (mode == "smoke-sync") logger = std::make_shared<yimingcode::SyncLogger>("review", yimingcode::LogLevel::value::DEBUG, fmt, sinks);
        else logger = std::make_shared<yimingcode::AsyncLogger>("review", yimingcode::LogLevel::value::DEBUG, fmt, sinks, yimingcode::AsyncType::ASYNC_SAFE);
        std::vector<std::thread> producers;
        for (int t = 0; t < 4; ++t) producers.emplace_back([&,t] {
            for (int i = 0; i < 1000; ++i) LOG_INFO(logger, "thread=%d item=%d", t, i);
        });
        for (auto& t : producers) t.join();
        logger.reset();
        std::cout << mode << " expected=4000 actual=" << sink->lines << '\n';
        return sink->lines == 4000 ? 0 : 1;
    }
    if (mode == "io-sync" || mode == "io-async") {
        auto sink = std::make_shared<yimingcode::FileSink>("/dev/full");
        auto fmt = std::make_shared<yimingcode::Formatter>("%m%n");
        std::vector<yimingcode::LogSink::ptr> sinks{sink};
        std::string payload(65536, 'X');
        if (mode == "io-sync") {
            yimingcode::SyncLogger logger("review", yimingcode::LogLevel::value::DEBUG, fmt, sinks);
            try { LOG_INFO((&logger), "%s", payload.c_str()); }
            catch (const std::exception& e) { std::cout << "sync caught: " << e.what() << '\n'; return 0; }
            return 1;
        }
        auto logger = std::make_shared<yimingcode::AsyncLogger>("review", yimingcode::LogLevel::value::DEBUG, fmt, sinks, yimingcode::AsyncType::ASYNC_SAFE);
        LOG_INFO(logger, "%s", payload.c_str());
        logger.reset();
        return 0;
    }
    if (mode == "retained-sink") {
        auto sink = std::make_shared<yimingcode::FileSink>("retained.log");
        auto fmt = std::make_shared<yimingcode::Formatter>("%m%n");
        std::vector<yimingcode::LogSink::ptr> sinks{sink};
        auto logger = std::make_shared<yimingcode::AsyncLogger>("review", yimingcode::LogLevel::value::DEBUG, fmt, sinks, yimingcode::AsyncType::ASYNC_SAFE);
        LOG_INFO(logger, "%s", "persist-me");
        logger.reset();
        std::ifstream before("retained.log", std::ios::binary | std::ios::ate);
        std::cout << "after logger destruction, visible bytes=" << before.tellg() << '\n';
        sink->flush();
        std::ifstream after("retained.log", std::ios::binary | std::ios::ate);
        std::cout << "after explicit sink flush, visible bytes=" << after.tellg() << '\n';
        return 0;
    }
    return 2;
}
