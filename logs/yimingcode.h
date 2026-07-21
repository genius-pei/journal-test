#ifndef __M_YIMINGCODE_H__
#define __M_YIMINGCODE_H__
#include"logger.hpp"

namespace yimingcode
{
    // 获取指定日志器的全局接口
    inline Logger::ptr getLogger(const std::string& name)
    {
        return LoggerManager::getInstance().getLogger(name);
    }
    
    inline Logger::ptr rootLogger()
    {
        return LoggerManager::getInstance().rootLogger();
    }
}

// ========== 宏定义 ==========

// 方式1：带 logger 对象的宏
#define LOG_DEBUG(logger, fmt, ...) \
    logger->debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(logger, fmt, ...) \
    logger->info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(logger, fmt, ...) \
    logger->warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(logger, fmt, ...) \
    logger->error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_FATAL(logger, fmt, ...) \
    logger->fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

// 方式2：使用 root logger 的宏（直接使用 DEBUG、INFO 等）
#define DEBUG(fmt, ...) \
    yimingcode::rootLogger()->debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define INFO(fmt, ...) \
    yimingcode::rootLogger()->info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define WARN(fmt, ...) \
    yimingcode::rootLogger()->warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ERROR(fmt, ...) \
    yimingcode::rootLogger()->error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define FATAL(fmt, ...) \
    yimingcode::rootLogger()->fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif