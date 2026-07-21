
//完成日志器模块
//1.抽象日志器基类别
//2.派生成不同的子类
#ifndef __MY_LOGGER_H__
#define __MY_LOGGER_H__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif     

#include"util.hpp"
#include"level.hpp"
#include"format.hpp"
#include"sink.hpp"
#include<atomic>
#include<cstdarg>
#include <mutex>
#include<assert.h>
#include"looper.hpp"
#include<unordered_map>


namespace yimingcode
{
    class Logger
    {
        public:
            using ptr =std::shared_ptr<Logger>;

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


            const std::string& name()
            {
                return _logger_name;
            }

            //构造日志消息工程并格式化，然后落地输出  
            void debug(const std::string& file, size_t line , const std::string& fmt , ...)
            {
                //通过传入的参数构造出一个日志消息对象，对日志进行格式化
                //1.判断日志是否达到了输出等级
                if(LogLevel::value::DEBUG< _limit_level)
                {
                    return ;
                }
                //2.对fmt格式化最进行字符串组织
                va_list ap;
                va_start(ap,fmt);
                char* res;
                int ret =vasprintf(&res,fmt.c_str(),ap);
                if(ret == -1)
                {
                    std::cout<<"vasprintf failed!!\n";
                    return;
                }
                va_end(ap);
                serialize(LogLevel::value::DEBUG,file,line,res);
                free(res);
            }
            void info(const std::string& file, size_t line , const std::string& fmt , ...)
            {
                //通过传入的参数构造出一个日志消息对象，对日志进行格式化
                //1.判断日志是否达到了输出等级
                if(LogLevel::value::INFO< _limit_level)
                {
                    return ;
                }
                //2.对fmt格式化最进行字符串组织
                va_list ap;
                va_start(ap,fmt);
                char* res;
                int ret =vasprintf(&res,fmt.c_str(),ap);
                if(ret == -1)
                {
                    std::cout<<"vasprintf failed!!\n";
                    return;
                }
                va_end(ap);
                serialize(LogLevel::value::INFO,file,line,res);
                free(res);
            }
            void warn(const std::string& file, size_t line , const std::string& fmt , ...)
            {
                //通过传入的参数构造出一个日志消息对象，对日志进行格式化
                //1.判断日志是否达到了输出等级
                if(LogLevel::value::WARN< _limit_level)
                {
                    return ;
                }
                //2.对fmt格式化最进行字符串组织
                va_list ap;
                va_start(ap,fmt);
                char* res;
                int ret =vasprintf(&res,fmt.c_str(),ap);
                if(ret == -1)
                {
                    std::cout<<"vasprintf failed!!\n";
                    return;
                }
                va_end(ap);
                serialize(LogLevel::value::WARN,file,line,res);
                free(res);
            }
            void error(const std::string& file, size_t line , const std::string& fmt , ...)
            {
                //通过传入的参数构造出一个日志消息对象，对日志进行格式化
                //1.判断日志是否达到了输出等级
                if(LogLevel::value::ERROR< _limit_level)
                {
                    return ;
                }
                //2.对fmt格式化最进行字符串组织
                va_list ap;
                va_start(ap,fmt);
                char* res;
                int ret =vasprintf(&res,fmt.c_str(),ap);
                if(ret == -1)
                {
                    std::cout<<"vasprintf failed!!\n";
                    return;
                }
                va_end(ap);
                serialize(LogLevel::value::ERROR,file,line,res);
                free(res);
            }
            void fatal(const std::string& file, size_t line , const std::string& fmt , ...)
            {
                //通过传入的参数构造出一个日志消息对象，对日志进行格式化
                //1.判断日志是否达到了输出等级
                if(LogLevel::value::FATAL< _limit_level)
                {
                    return ;
                }
                //2.对fmt格式化最进行字符串组织
                va_list ap;
                va_start(ap,fmt);
                char* res;
                int ret =vasprintf(&res,fmt.c_str(),ap);
                if(ret == -1)
                {
                    std::cout<<"vasprintf failed!!\n";
                    return;
                }
                va_end(ap);
                serialize(LogLevel::value::FATAL,file,line,res);
                free(res);
            }
            
    protected:
        //抽象接口完成实际的落地输出，不同的日志器有不同的实际落地方式
        void serialize(LogLevel::value level, const std::string& file, size_t line, char* str)
        {
            //  使用传入的 level 参数
            LogMsg msg(level, line, file, _logger_name, str);  // ← 改为 level
            // 4.通过格式化工具对LogMsg进行格式化，得到格式化后的字符串
            std::stringstream ss;
            _formatter->format(ss, msg);
            // 5，日志落地
            log(ss.str().c_str(), ss.str().size());
        }
            virtual void log(const char* data, size_t len) {
            // 默认实现：什么都不做
            }
      
    protected:
        std::mutex _mutex;//互斥锁
        std::string _logger_name;
        LogLevel::value _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;

    };

    class SyncLogger:public Logger
    {
        public:
        SyncLogger(const std::string& logger_name,
           LogLevel::value level,
           const Formatter::ptr& formatter,
           const std::vector<LogSink::ptr>& sinks)
    : Logger(logger_name, level, formatter, sinks)
        {}
        protected:
        //同步日志器，是将日志直接通过落地模块进行日志落地
        void log(const char* data,size_t len)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if(_sinks.empty()) return;
            for(auto& sink : _sinks)
            {
                sink->log(data,len);
            }
        }

    };
    
    class AsyncLogger: public Logger
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
        {
        }
        
        ~AsyncLogger()
        {
            if(_looper)
            {
                _looper->stop();
            }
        }
        
        void log(const char* data, size_t len) override
        {
            if(_looper)
            {
                _looper->push(data, len);
            }
        }
        
        // 实际落地函数（将缓冲区数据落地）
        void realLog(Buffer &buf)
        {
            if(_sinks.empty()) return;
            
        
            const char* data = buf.begin();
            size_t len = buf.readAbleSize();
            
            if(len == 0) return;
            
            for(auto& sink : _sinks)
            {
                if(sink)
                {
                    sink->log(data, len);
                }
            }
        }
        
    private:
        ASyncLogLooper::ptr _looper;
};
    
    enum class LoggerType
    {
        LOGGER_SYNC,     //同步日志器
        LOGGER_ASYNC    //异步日志器
    };
    //抽象一个建造者模式，简化用户使用复杂度
    class LoggerBuilder
    {
        public:
        LoggerBuilder()
        :_logger_type(LoggerType::LOGGER_SYNC)
        ,_limit_level(LogLevel::value::DEBUG)
        ,_looper_type(AsyncType::ASYNC_SAFE)
        {

        }
        void buildLoggerType(LoggerType type)
        {
            _logger_type=type;
        }
        void buildEnableUnSafeAsync()
        {
            _looper_type=AsyncType::ASYNC_UNSAFE;
        }
        void buildLoggerName(const std::string& name)
        {
            _logger_name=name;
        }
        void buildLoggerLevel(LogLevel::value level)
        {
            _limit_level=level;
        }
        void buildFormatter(const std::string& pattern)
        {
            _formatter=std::make_shared<Formatter>(pattern); 
        }
        template<typename SinkType,typename... Args>
        void buildSink(Args&&... args)
        {
            LogSink::ptr psink =SinkFactory::create<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(psink);
            
        }
        virtual Logger::ptr build() = 0;

        protected:
        AsyncType _looper_type;
        LoggerType  _logger_type;
        std::string _logger_name;   
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class LocalLoggerBuilder: public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(_logger_name.empty() == false);
            
            if(_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            
            if(_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            
            //  异步日志器
            if(_logger_type == LoggerType::LOGGER_ASYNC)
            {
                return std::make_shared<AsyncLogger>(
                    _logger_name,
                    _limit_level.load(),  // atomic 需要 load()
                    _formatter,
                    _sinks,
                    _looper_type
                );
            }
            
            //  同步日志器
            return std::make_shared<SyncLogger>(
                _logger_name,
                _limit_level.load(),  //  atomic 需要 load()
                _formatter,
                _sinks
            );
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
            void addLogger(Logger::ptr& logger)
            {
                if(hasLogger(logger->name()))
                {
                    return;
                };
                std::unique_lock<std::mutex> lock(_mutex);
                _loggers.insert(std::make_pair(logger->name(),logger));
            }
            bool hasLogger(const std::string& name)
            {
               std::unique_lock<std::mutex> lock(_mutex);
               auto it =_loggers.find(name);
               if(it == _loggers.end())
               {
                    return false;
               }
                    return true;
            }
            Logger::ptr getLogger(const std::string &name)
            {
                    std::unique_lock<std::mutex> lock(_mutex);
                    auto it =_loggers.find(name);
                    if(it == _loggers.end())
                    {
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
                std::unique_ptr<yimingcode::LocalLoggerBuilder> builder(new yimingcode::LocalLoggerBuilder());
                builder->buildLoggerName("root");
                _root_logger =builder->build();
                _loggers.insert(std::make_pair("root",_root_logger));
            }
        private:
        std::mutex _mutex;
        Logger::ptr _root_logger;//默认日志器
        std::unordered_map<std::string,Logger::ptr> _loggers;
    };
    //设计一个全局日志器的建造者
    class GlobalLoggerBuilder: public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(_logger_name.empty() == false);
            
            if(_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            
            if(_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            
            Logger::ptr logger;
            //  异步日志器
            if(_logger_type == LoggerType::LOGGER_ASYNC)
            {
                logger = std::make_shared<AsyncLogger>(
                    _logger_name,
                    _limit_level.load(),  // atomic 需要 load()
                    _formatter,
                    _sinks,
                    _looper_type
                );
            }
            else
            {
            //  同步日志器
                logger = std::make_shared<SyncLogger>(_logger_name,_limit_level.load(),_formatter, _sinks);

            }
                LoggerManager::getInstance().addLogger(logger);
                return logger;
        }
    };





}


#endif