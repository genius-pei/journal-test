# yimingcode 日志库 — 技术规格

## 1. 概述

yimingcode 是一个基于 C++11 标准库的**头文件 only** 日志库，采用模块化设计，核心代码位于 `logs/` 目录下。

- 语言标准：C++11
- 依赖：仅 C++ 标准库 + POSIX（`vasprintf`, `localtime_r`, `mkdir`）
- 命名空间：`yimingcode`
- 线程模型：多线程安全，基于 `std::mutex`

---

## 2. 日志级别

定义在 `logs/level.hpp`，枚举 `yimingcode::LogLevel::value`：

| 级别 | 值 | 说明 |
|------|----|------|
| UNKNOWN | 0 | 未知 |
| DEBUG   | 1 | 调试 |
| INFO    | 2 | 信息 |
| WARN    | 3 | 警告 |
| ERROR   | 4 | 错误 |
| FATAL   | 5 | 致命 |
| OFF     | 6 | 关闭 |

通过 `setLimitLevel()` 设置过滤级别，低于该级别的日志将被丢弃。

---

## 3. 日志消息

定义在 `logs/message.hpp`，结构体 `yimingcode::LogMsg`：

| 字段 | 类型 | 说明 |
|------|------|------|
| `_ctime` | `time_t` | 日志产生时间 |
| `_line` | `uint32_t` | 源文件行号 |
| `_thread_id` | `uint64_t` | 线程 ID |
| `_payload` | `std::string` | 日志内容 |
| `_level` | `LogLevel::value` | 日志级别 |
| `_name` | `std::string` | 日志器名称 |
| `_file` | `std::string` | 源文件名 |

---

## 4. 格式化引擎

定义在 `logs/format.hpp`，类 `yimingcode::Formatter`。

### 4.1 格式字符串语法

格式字符串由普通字符和格式占位符组成。普通字符原样输出。

### 4.2 格式占位符

| 占位符 | 含义 | 示例输出 |
|--------|------|----------|
| `%d{format}` | 时间，format 为 strftime 格式 | `[14:30:05]` |
| `%t` | 线程 ID | `[140736292237568]` |
| `%c` | 日志器名称 | `root` |
| `%f` | 文件名 | `main.cc` |
| `%l` | 行号 | `42` |
| `%p` | 日志级别 | `DEBUG` |
| `%T` | 制表符 | `\t` |
| `%m` | 消息正文 | `hello world` |
| `%n` | 换行 | `\n` |
| `%%` | 转义 % | `%` |

### 4.3 默认格式

```
[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n
```

示例输出：

```
[14:30:05][140736292237568][root][main.cc:42][DEBUG]  hello world
```

---

## 5. 输出 Sink

定义在 `logs/sink.hpp`，所有 Sink 继承自 `yimingcode::LogSink`。

### 5.1 内置 Sink

| 类 | 功能 |
|----|------|
| `StdoutSink` | 输出到标准输出 |
| `FileSink` | 输出到指定文件 |
| `RollBySizeSink` | 按文件大小滚动 |
| `NullSink` | 空输出（丢弃） |

### 5.2 RollBySizeSink

- 构造参数：文件名前缀、最大字节数
- 当文件大小超过 `_roll_size` 时，关闭当前文件，创建新文件
- 新文件命名规则：`{prefix}{timestamp}-{index}.log`

### 5.3 自定义 Sink

继承 `yimingcode::LogSink`，实现 `log()` 与 `dump()` 方法。参见 `extend/` 中的 `RollByTimeSink` 示例。

### 5.4 SinkFactory

变参模板工厂，封装 `std::make_shared<T>(args...)`：

```cpp
builder.buildSinker<yimingcode::FileSink>("app.log");
```

---

## 6. 日志器

定义在 `logs/logger.hpp`。

### 6.1 Logger（抽象基类）

提供日志方法：

```cpp
void debug(const char* fmt, ...);
void info(const char* fmt, ...);
void warn(const char* fmt, ...);
void error(const char* fmt, ...);
void fatal(const char* fmt, ...);
```

内部使用 `vasprintf` 格式化，`free` 释放。

### 6.2 SyncLogger

每次调用日志方法时：
1. 加锁 `_mutex`
2. 格式化消息
3. 遍历所有 Sink 调用 `log(msg)`
4. （可选）调用 `dump()` 落盘

### 6.3 AsyncLogger

1. 格式化消息
2. 将 `LogMsg` 推入 `ASyncLogLooper` 的缓冲区
3. 后台线程消费缓冲区，批量写入 Sink

### 6.4 构建器（Builder）

| 类 | 说明 |
|----|------|
| `LoggerBuilder` | 抽象构建器基类 |
| `LocalLoggerBuilder` | 构建独立日志器，不注册到全局管理器 |
| `GlobalLoggerBuilder` | 构建日志器并注册到 `LoggerManager` |

构建方法链：

```cpp
auto logger = GlobalLoggerBuilder()
    .buildLoggerName("name")          // 日志器名称
    .buildLimitLevel(LogLevel::DEBUG) // 级别过滤
    .buildSinker<FileSink>("path")    // 添加 Sink
    .buildSinker<StdoutSink>()        // 可添加多个
    .buildFormatter(format_str)       // 自定义格式（可选）
    .buildLoggerType<AsyncLogger>()   // 同步/异步
    .build();                         // 构建
```

### 6.5 LoggerManager（单例）

- `getInstance()` — 获取单例
- `getLogger(name)` — 获取或创建日志器
- `rootLogger()` — 获取 root 日志器
- 首次调用自动创建名为 "root" 的同步日志器（`StdoutSink` + DEBUG 级别）

---

## 7. 异步循环器

定义在 `logs/looper.hpp`，类 `yimingcode::ASyncLogLooper`。

### 7.1 双缓冲模型

- `_pro_buf`：生产者缓冲区，日志器写入
- `_con_buf`：消费者缓冲区，后台线程处理

当 `_pro_buf` 满（或主动刷新）时，交换缓冲区。

### 7.2 缓冲区策略

定义在 `logs/buffer.hpp`，类 `yimingcode::Buffer`。

- 初始容量：1MB
- 扩容策略：加倍扩容，上限 8MB，之后线性增长（每次 1MB）
- 模式：
  - `SAFE`：缓冲区满时阻塞等待
  - `UNSAFE`：缓冲区满时自动扩容（无界）

### 7.3 线程模型

- 后台线程通过 `std::condition_variable` 等待
- 收到通知后交换缓冲区、调用 Sink 回调
- 析构时刷空缓冲区

---

## 8. 宏 API

定义在 `logs/yimingcode.h`。

### 8.1 指定日志器

```cpp
LOG_DEBUG(logger, fmt, ...)
LOG_INFO(logger, fmt, ...)
LOG_WARN(logger, fmt, ...)
LOG_ERROR(logger, fmt, ...)
LOG_FATAL(logger, fmt, ...)
```

### 8.2 全局默认（rootLogger）

```cpp
DEBUG(fmt, ...)    // 等价于 LOG_DEBUG(rootLogger(), ...)
INFO(fmt, ...)
WARN(fmt, ...)
ERROR(fmt, ...)
FATAL(fmt, ...)
```

所有宏自动携带 `__FILE__`、`__LINE__`、`__FUNCTION__`。

---

## 9. 工具函数

定义在 `logs/util.hpp`，命名空间 `yimingcode::util`。

| API | 说明 |
|-----|------|
| `Date::now()` | 返回当前 `time_t` |
| `file::exists(path)` | 判断文件是否存在 |
| `file::path(path)` | 返回路径中的目录部分 |
| `file::create_directory(path)` | 创建目录（含父目录） |

---

## 10. 构建要求

- 编译器：支持 C++11 的 GCC / Clang
- 系统：Linux / macOS（依赖 POSIX API）
- 链接：`-lpthread`

---

## 11. 性能指标

bench/ 目录下提供性能基准测试，测试指标：

| 指标 | 说明 |
|------|------|
| 消息数 | 总计写入消息数 |
| 线程数 | 并发写入线程数 |
| 单条大小 | 每条消息的平均字节数 |
| 总耗时 | 所有线程完成写入的壁钟时间 |
| 吞吐量 | 每秒写入消息数（msg/s）和带宽（MB/s） |

---

## 12. 扩展指南

### 12.1 自定义 Sink

```cpp
class MySink : public yimingcode::LogSink {
public:
    void log(const char* data, size_t len) override {
        // 自定义输出逻辑
    }
    void dump() override {
        // 刷盘逻辑（可选）
    }
};

// 使用
builder.buildSinker<MySink>(args...);
```

### 12.2 自定义日志器

继承 `yimingcode::Logger`，实现 `log()` 方法。

---

## 13. 注意事项

- `FATAL` 级别日志不终止程序（与常见日志库约定不同）
- `vasprintf` 为 POSIX 扩展，Windows 下需替代实现
- 宏名称 `DEBUG`/`INFO`/`WARN`/`ERROR`/`FATAL` 可能与其他库冲突，可使用带 `LOG_` 前缀的版本
