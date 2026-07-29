#ifndef YIMINGCODE_LOGGER_H__
#define YIMINGCODE_LOGGER_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "util.hpp"
#include "level.hpp"
#include "format.hpp"
#include "sink.hpp"
#include <atomic>
#include <cstdarg>
#include <mutex>
#include "looper.hpp"
#include <unordered_map>
#include <stdexcept>

namespace yimingcode
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;

        Logger(const std::string& logger_name,
               LogLevel::value level,
               const Formatter::ptr& formatter,
               const std::vector<LogSink::ptr>& sinks)
            : _logger_name(logger_name)
            , _limit_level(level)
            , _formatter(formatter)
            , _sinks(sinks)
        {}

        virtual ~Logger() = default;

        const std::string& name() const
        {
            return _logger_name;
        }

        void debug(const std::string& file, size_t line, const std::string& fmt, ...)
        {
            if (LogLevel::value::DEBUG < _limit_level) return;
            va_list ap;
            va_start(ap, fmt);
            logV(LogLevel::value::DEBUG, file, line, fmt, ap);
            va_end(ap);
        }

        void info(const std::string& file, size_t line, const std::string& fmt, ...)
        {
            if (LogLevel::value::INFO < _limit_level) return;
            va_list ap;
            va_start(ap, fmt);
            logV(LogLevel::value::INFO, file, line, fmt, ap);
            va_end(ap);
        }

        void warn(const std::string& file, size_t line, const std::string& fmt, ...)
        {
            if (LogLevel::value::WARN < _limit_level) return;
            va_list ap;
            va_start(ap, fmt);
            logV(LogLevel::value::WARN, file, line, fmt, ap);
            va_end(ap);
        }

        void error(const std::string& file, size_t line, const std::string& fmt, ...)
        {
            if (LogLevel::value::ERROR < _limit_level) return;
            va_list ap;
            va_start(ap, fmt);
            logV(LogLevel::value::ERROR, file, line, fmt, ap);
            va_end(ap);
        }

        void fatal(const std::string& file, size_t line, const std::string& fmt, ...)
        {
            if (LogLevel::value::FATAL < _limit_level) return;
            va_list ap;
            va_start(ap, fmt);
            logV(LogLevel::value::FATAL, file, line, fmt, ap);
            va_end(ap);
        }

    protected:
        void logV(LogLevel::value level, const std::string& file, size_t line,
                  const std::string& fmt, va_list ap)
        {
            char* res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1) return;
            serialize(level, file, line, res);
            free(res);
        }

        void serialize(LogLevel::value level, const std::string& file, size_t line, char* str)
        {
            LogMsg msg(level, line, file, _logger_name, str);
            std::stringstream ss;
            _formatter->format(ss, msg);
            auto body = ss.str();
            log(body.c_str(), body.size());
        }

        virtual void log(const char* data, size_t len) {}

    protected:
        std::mutex _mutex;
        std::string _logger_name;
        LogLevel::value _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string& logger_name,
                   LogLevel::value level,
                   const Formatter::ptr& formatter,
                   const std::vector<LogSink::ptr>& sinks)
            : Logger(logger_name, level, formatter, sinks)
        {}

    protected:
        void log(const char* data, size_t len) override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_sinks.empty()) return;
            for (auto& sink : _sinks) {
                sink->log(data, len);
            }
        }
    };

    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string& logger_name,
                    LogLevel::value level,
                    const Formatter::ptr& formatter,
                    const std::vector<LogSink::ptr>& sinks,
                    AsyncType looper_type)
            : Logger(logger_name, level, formatter, sinks)
            , _looper(std::make_shared<ASyncLogLooper>(
                  std::bind(&AsyncLogger::realLog, this, std::placeholders::_1),
                  looper_type))
        {}

        ~AsyncLogger()
        {
            if (_looper) {
                _looper->stop();
            }
        }

        void log(const char* data, size_t len) override
        {
            if (_looper) {
                _looper->push(data, len);
            }
        }

        void realLog(Buffer &buf)
        {
            if (_sinks.empty()) return;
            const char* data = buf.begin();
            size_t len = buf.readAbleSize();
            if (len == 0) return;
            for (auto& sink : _sinks) {
                if (sink) {
                    sink->log(data, len);
                }
            }
        }

    private:
        ASyncLogLooper::ptr _looper;
    };

    enum class LoggerType
    {
        LOGGER_SYNC,
        LOGGER_ASYNC
    };

    class LoggerBuilder
    {
    public:
        LoggerBuilder()
            : _logger_type(LoggerType::LOGGER_SYNC)
            , _limit_level(LogLevel::value::DEBUG)
            , _looper_type(AsyncType::ASYNC_SAFE)
        {}

        void buildLoggerType(LoggerType type) { _logger_type = type; }
        void buildEnableUnSafeAsync() { _looper_type = AsyncType::ASYNC_UNSAFE; }
        void buildLoggerName(const std::string& name) { _logger_name = name; }
        void buildLoggerLevel(LogLevel::value level) { _limit_level = level; }

        void buildFormatter(const std::string& pattern)
        {
            _formatter = std::make_shared<Formatter>(pattern);
        }

        template<typename SinkType, typename... Args>
        void buildSink(Args&&... args)
        {
            _sinks.push_back(SinkFactory::create<SinkType>(std::forward<Args>(args)...));
        }

        virtual Logger::ptr build() = 0;

    protected:
        AsyncType _looper_type;
        LoggerType _logger_type;
        std::string _logger_name;
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            if (_logger_name.empty()) {
                throw std::invalid_argument("Logger name must not be empty");
            }
            if (!_formatter) {
                _formatter = std::make_shared<Formatter>();
            }
            if (_sinks.empty()) {
                buildSink<StdoutSink>();
            }
            if (_logger_type == LoggerType::LOGGER_ASYNC) {
                return std::make_shared<AsyncLogger>(
                    _logger_name, _limit_level.load(), _formatter, _sinks, _looper_type);
            }
            return std::make_shared<SyncLogger>(
                _logger_name, _limit_level.load(), _formatter, _sinks);
        }
    };

    class LoggerManager
    {
    public:
        static LoggerManager& getInstance()
        {
            static LoggerManager eton;
            return eton;
        }

        void addLogger(const Logger::ptr& logger)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_loggers.find(logger->name()) != _loggers.end()) {
                return;
            }
            _loggers.insert(std::make_pair(logger->name(), logger));
        }

        bool hasLogger(const std::string& name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _loggers.find(name) != _loggers.end();
        }

        Logger::ptr getLogger(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _loggers.find(name);
            if (it == _loggers.end()) {
                return Logger::ptr();
            }
            return it->second;
        }

        Logger::ptr rootLogger()
        {
            return _root_logger;
        }

    private:
        LoggerManager()
        {
            std::unique_ptr<LocalLoggerBuilder> builder(new LocalLoggerBuilder());
            builder->buildLoggerName("root");
            _root_logger = builder->build();
            _loggers.insert(std::make_pair("root", _root_logger));
        }

    private:
        std::mutex _mutex;
        Logger::ptr _root_logger;
        std::unordered_map<std::string, Logger::ptr> _loggers;
    };

    class GlobalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            if (_logger_name.empty()) {
                throw std::invalid_argument("Logger name must not be empty");
            }
            if (!_formatter) {
                _formatter = std::make_shared<Formatter>();
            }
            if (_sinks.empty()) {
                buildSink<StdoutSink>();
            }

            Logger::ptr logger;
            if (_logger_type == LoggerType::LOGGER_ASYNC) {
                logger = std::make_shared<AsyncLogger>(
                    _logger_name, _limit_level.load(), _formatter, _sinks, _looper_type);
            } else {
                logger = std::make_shared<SyncLogger>(
                    _logger_name, _limit_level.load(), _formatter, _sinks);
            }
            LoggerManager::getInstance().addLogger(logger);
            return logger;
        }
    };
}


#endif