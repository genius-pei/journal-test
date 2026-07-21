#include <iostream>
#include"../logs/yimingcode.h"



//扩展一个以时间作为切换的标准的日志落地模块
enum class TimeGap
{
    GAP_SECOND,
    GAP_MINUTE,
    GAP_HOUR,
    GAP_DAY,
};

class RollByTimeSink : public yimingcode::LogSink{
public:
    // 构造时传入文件名，打开文件，将操作句柄管理
    RollByTimeSink(const std::string &basename,TimeGap gap_type)
    :_basename(basename)
    {
        switch(gap_type)
        {
            case TimeGap::GAP_SECOND: _gap_size=1;break;
             case TimeGap::GAP_MINUTE: _gap_size=60;break;
              case TimeGap::GAP_HOUR: _gap_size=3600;break;
               case TimeGap::GAP_DAY: _gap_size=3600*24;break;
        }
        _cur_gap=_gap_size==1?yimingcode::util::Date::now():yimingcode::util::Date::now()%_gap_size;//获取当前是第几个时间段
        std::string filename =createNewFile();
        yimingcode::util::file::create_directory(yimingcode::util::file::path(filename));
          _ofs.open( filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
    }
    
    
    //将日志消息写入到标准输出，判断当前时间是否当前文件的时间段，不是则切换时间
    void log(const char* data,size_t len) {
        time_t cur=yimingcode::util::Date::now();
        if((cur % _gap_size)!=_cur_gap)
        {
            _ofs.close();
            std::string filename =createNewFile();
            _ofs.open( filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }
        _ofs.write(data,len);
        assert(_ofs.good());
    }

private:
     std::string createNewFile()
    {
        time_t t = yimingcode::util::Date::now();
        struct tm lt;
        localtime_r(&t, &lt);
        
        std::stringstream filename;
        filename << _basename
                 << (lt.tm_year + 1900)
                 << (lt.tm_mon + 1)
                 << lt.tm_mday
                 << lt.tm_hour
                 << lt.tm_min
                 << lt.tm_sec
                 << ".log";
        return filename.str();
    }

   
private:
    std::string _basename;
    std::ofstream _ofs;
    size_t _cur_gap;//当前是第几个时间段
    size_t _gap_size;
};

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
    yimingcode::Logger::ptr logger =builder->build();
    size_t cur =yimingcode::util::Date::now();
    while(yimingcode::util::Date::now()<cur+5)
    {
        FATAL("这是一个测试日志");
        sleep(1);
    }
    
     return 0;
}