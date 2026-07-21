
     #ifndef __M_LEVEL_H__
    #define __M_LEVEL_H__

    #include<iostream>
    
    
    namespace yimingcode
    {
        class LogLevel
        {
            public:
            enum class value//定义枚举类，枚举出日志等级
            {
                UNKNOWN = 0,
                DEBUG,
                INFO,
                WARN,
                ERROR,
                FATAL,
                OFF
            };
            static const char *toString(LogLevel::value level)
            {
                switch(level)
                {
                    case LogLevel::value::DEBUG: return "DEBUG";
                    case LogLevel::value::INFO: return "INFO";
                    case LogLevel::value::WARN: return "WARN";
                    case LogLevel::value::ERROR: return "ERROR";
                    case LogLevel::value::FATAL: return "FATAL";
                     case LogLevel::value::OFF: return "OFF";
                 
                }
                return "UNKNOWN";
                

            }
        };
    }
  #endif