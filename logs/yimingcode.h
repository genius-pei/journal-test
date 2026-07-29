#ifndef YIMINGCODE_YIMINGCODE_H__
#define YIMINGCODE_YIMINGCODE_H__

#include "logger.hpp"

namespace yimingcode
{
    inline Logger::ptr getLogger(const std::string& name)
    {
        return LoggerManager::getInstance().getLogger(name);
    }

    inline Logger::ptr rootLogger()
    {
        return LoggerManager::getInstance().rootLogger();
    }
}

#define LOG_DEBUG(logger, fmt, ...) \
    (logger)->debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(logger, fmt, ...) \
    (logger)->info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(logger, fmt, ...) \
    (logger)->warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(logger, fmt, ...) \
    (logger)->error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_FATAL(logger, fmt, ...) \
    (logger)->fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#ifndef YIMINGCODE_DISABLE_SHORT_MACROS
#ifndef DEBUG
#define DEBUG(fmt, ...) \
    yimingcode::rootLogger()->debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

#ifndef INFO
#define INFO(fmt, ...) \
    yimingcode::rootLogger()->info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

#ifndef WARN
#define WARN(fmt, ...) \
    yimingcode::rootLogger()->warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

#ifndef ERROR
#define ERROR(fmt, ...) \
    yimingcode::rootLogger()->error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

#ifndef FATAL
#define FATAL(fmt, ...) \
    yimingcode::rootLogger()->fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#endif

#endif