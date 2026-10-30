#include "logs/yimingcode.h"
#include <future>
#include <stdexcept>
#include <iostream>

class FailedSink : public yimingcode::LogSink
{
public:
    void log(const char*, size_t) override
    {
        throw std::runtime_error("test sink failure");
    }
};

class NotifySink : public yimingcode::LogSink
{
public:
    std::promise<void> written;
    void log(const char*, size_t) override { written.set_value(); }
};

int main()
{
    auto sink = std::make_shared<NotifySink>();
    auto done = sink->written.get_future();
    auto fmt = std::make_shared<yimingcode::Formatter>("%m%n");
    std::vector<yimingcode::LogSink::ptr> sinks{std::make_shared<FailedSink>(), sink};
    auto logger = std::make_shared<yimingcode::AsyncLogger>("test",
        yimingcode::LogLevel::value::DEBUG, fmt, sinks, yimingcode::AsyncType::ASYNC_SAFE);
    LOG_INFO(logger, "%s", "still delivered");
    if (done.wait_for(std::chrono::seconds(2)) != std::future_status::ready) return 1;
    logger.reset();

    yimingcode::ASyncLogLooper looper([](yimingcode::Buffer&) { throw 1; },
        yimingcode::AsyncType::ASYNC_UNSAFE);
    looper.push("a", 1);
    looper.stop();
    if (looper.errorCount() != 1) return 2;
    looper.push("b", 1);
    if (looper.errorCount() != 1) return 3;
    std::cout << "error isolation and unknown exception passed\n";
}
