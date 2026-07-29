# yimingcode — 高性能 C++ 日志库

yimingcode 是一个**头文件 only**的 C++11 日志库，支持同步/异步日志、多级别过滤、灵活的输出格式和可扩展的 Sink 架构。

## 特性

- **头文件 only** — 无需编译库文件，直接 `#include` 即可使用
- **同步/异步日志** — `SyncLogger` 直接写入，`AsyncLogger` 通过生产者-消费者模型批量落盘
- **日志级别** — `DEBUG / INFO / WARN / ERROR / FATAL / OFF`
- **可扩展 Sink** — 内置 `StdoutSink`、`FileSink`、`RollBySizeSink`；支持自定义 Sink
- **自定义格式** — 通过格式字符串控制输出，如 `%d %t %c %f:%l %p %m %n`
- **线程安全** — 基于 `std::mutex` 的细粒度锁
- **全局管理器** — 通过 `LoggerManager` 按名称管理多个日志器，自动创建 root 日志器
- **宏封装** — `LOG_DEBUG` / `LOG_INFO` / `LOG_WARN` / `LOG_ERROR` / `LOG_FATAL` 便捷宏

## 目录结构

```
├── logs/             核心库（头文件）
│   ├── level.hpp     日志级别枚举
│   ├── util.hpp      日期与文件工具
│   ├── message.hpp   日志消息结构
│   ├── format.hpp    格式化引擎
│   ├── buffer.hpp    异步缓冲区
│   ├── looper.hpp    异步循环器（生产者-消费者）
│   ├── sink.hpp      输出 Sink 实现
│   ├── logger.hpp    日志器核心（Logger / Builder / Manager）
│   └── yimingcode.h  公开 API 与宏
├── example/          使用示例
├── extend/           Sink 扩展示例（按时间滚动）
├── bench/            性能基准测试
└── pratice/          练习目录
```

## 快速开始

```cpp
#include "logs/yimingcode.h"

int main() {
    // 使用 rootLogger 直接输出
    DEBUG("hello %s", "yimingcode");

    // 构建自定义日志器
    auto logger = yimingcode::GlobalLoggerBuilder()
        .buildLoggerName("mylogger")
        .buildLimitLevel(yimingcode::LogLevel::DEBUG)
        .buildSinker<yimingcode::FileSink>("./app.log")
        .buildSinker<yimingcode::StdoutSink>()
        .buildLoggerType<yimingcode::AsyncLogger>()
        .build();

    LOG_DEBUG(logger, "this is a %s log", "debug");
    LOG_INFO(logger, "hello %s", "async");
}
```

编译：`g++ -std=c++11 main.cc -lpthread`

## 构建与运行

```bash
# 示例
cd example && make && ./test

# 扩展示例（按时间滚动）
cd extend && make && ./test

# 基准测试
cd bench && make && ./test
```

## 日志格式

默认格式：`[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n`

| 占位符 | 说明 |
|--------|------|
| `%d{fmt}` | 日期时间，fmt 为 strftime 格式 |
| `%t` | 线程 ID |
| `%c` | 日志器名称 |
| `%f` | 源文件名 |
| `%l` | 源文件行号 |
| `%p` | 日志级别 |
| `%T` | 制表符 |
| `%m` | 用户消息 |
| `%n` | 换行符 |

## 性能

在 bench/ 目录下运行基准测试，涵盖单线程/多线程的同步与异步场景。
