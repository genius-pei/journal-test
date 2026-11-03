# 高性能 C++ 日志库

[![C++ CI](https://github.com/genius-pei/journal-test/actions/workflows/ci.yml/badge.svg)](https://github.com/genius-pei/journal-test/actions/workflows/ci.yml)

一个 C++11 日志库，支持同步/异步日志、多级别过滤、灵活的输出格式和可扩展的 Sink 架构。

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
└── tests/            回归测试
```

## 快速开始

```cpp
#include "logs/yimingcode.h"

int main() {
    // 使用 rootLogger 直接输出
    DEBUG("hello %s", "yimingcode");

    // 构建自定义日志器
    yimingcode::GlobalLoggerBuilder builder;
    builder.buildLoggerName("mylogger");
    builder.buildLoggerLevel(yimingcode::LogLevel::value::DEBUG);
    builder.buildSink<yimingcode::FileSink>("./app.log");
    builder.buildSink<yimingcode::StdoutSink>();
    builder.buildLoggerType(yimingcode::LoggerType::LOGGER_ASYNC);
    auto logger = builder.build();
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

在 `bench/` 目录运行 `make && ./test`。基准覆盖同步与异步文件输出，并报告生产者完成时间、异步消费者排空时间、总耗时和吞吐量。

每组测试会验证输出文件包含预期的完整记录数，并报告实际文件字节数。异步总耗时包括生产者结束后停止并等待后台线程处理已接收缓冲区，以及文件流关闭；它不代表 `fsync` 或断电后的持久性。比较结果时请在相同机器和相同负载下重复运行，并记录编译器与系统信息。

基准输出写入 `bench/bench-output/`。

## 异步错误与时间滚动

异步写入异常不会逃出工作线程。某个 Sink 失败后仍会尝试本批次的其他 Sink，
随后向标准错误报告第一个异常，并将失败批次数加一；失败数据不重试，后续批次继续消费。
可通过 `AsyncLogger::errorCount()` 查询累计失败批次数。同步日志仍将写入异常交给调用方。
停止后不再接受新数据；停止会等待已经接收的数据完成消费，但不等同于文件流 flush 或 fsync。

时间滚动以 Unix 时间戳除以窗口秒数划分窗口，天窗口是固定 86400 秒，不表示本地自然日。

回归检查：在项目根目录执行 `bash tests/run.sh`（Linux、g++、Python 3）。

## 持续集成

GitHub Actions 在推送到 main、提交 Pull Request 或手动触发时，使用 Ubuntu 24.04 编译 example 和 extend，并执行 bash tests/run.sh。回归脚本同时编译、运行上面的 README 示例；顶部徽章显示最近运行状态。
