#include <iostream>
#include"../logs/yimingcode.h"
#include"roll_by_time_sink.hpp"

int main()
{


    std::unique_ptr<yimingcode::LoggerBuilder> builder(new yimingcode::GlobalLoggerBuilder());
    builder->buildLoggerLevel(yimingcode::LogLevel::value::WARN);
    builder->buildLoggerName("async_logger");
    builder->buildFormatter("[%c][%f%l]%m%n");
    builder->buildLoggerType(yimingcode::LoggerType::LOGGER_ASYNC);
    // builder->buildEnableUnSafeAsync();
    builder->buildSink<yimingcode::FileSink>("./logfile/async.log");
    builder->buildSink<yimingcode::StdoutSink>();
    builder->buildSink<RollByTimeSink>("./logfile/roll-async-by-timr.log",TimeGap::GAP_SECOND);
    yimingcode::Logger::ptr logger = builder->build();
    size_t cur = yimingcode::util::Date::now();
    while (yimingcode::util::Date::now() < cur + 5) {
        LOG_FATAL(logger, "这是一个测试日志");
        sleep(1);
    }
    
     return 0;
}