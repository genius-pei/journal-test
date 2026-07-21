#ifndef __M_MSG_H__
#define __M_MSG_H__

#include<iostream>
#include<string>
#include<thread>
#include"level.hpp"
#include"util.hpp"

namespace yimingcode
{
    struct LogMsg
    {
        time_t _ctime;          // 日志产生的时间戳
        LogLevel::value _level; // 日志等级
        size_t _line;           // 行号
        std::thread::id _tid;   // 线程id
        std::string _file;      // 原码文件名
        std::string _logger;    // 日志器名称
        std::string _payload;   // 有效消息数据

        // 构造函数 - 参数顺序匹配您的测试调用: (level, line, file, logger, msg)
        LogMsg(LogLevel::value level,
               size_t line,
               const std::string& file,
               const std::string& logger,
               const std::string& msg) :
            _ctime(util::Date::now()),
            _level(level),
            _line(line),
            _tid(std::this_thread::get_id()),
            _file(file),
            _logger(logger),
            _payload(msg)
        {
        }
    };
}

#endif