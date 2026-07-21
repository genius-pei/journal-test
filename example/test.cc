
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

void test_log(const std::string name)
{   
    INFO("%s","测试开始");
        yimingcode::Logger::ptr logger =yimingcode::LoggerManager::getInstance().getLogger(name);
        DEBUG( "%s", "测试日志");
        INFO( "%s", "测试日志");
        WARN("%s", "测试日志");
        ERROR("%s", "测试日志");
        FATAL( "%s", "测试日志");  
    INFO("%s","测试结束");
}


int main()
{
//     std::string logger_name = "sync_logger";
//     yimingcode::LogLevel::value limit = yimingcode::LogLevel::value::WARN;
//     yimingcode::Formatter::ptr fmt(new yimingcode::Formatter("[%d{%H:%M:%S}][%c][%f:%l][%p]%T%m%n"));
//     yimingcode::LogSink::ptr stdout_lsp =yimingcode::SinkFactory::create<yimingcode::StdoutSink>();
//     yimingcode::LogSink::ptr file_lsp = yimingcode::SinkFactory::create<yimingcode::FileSink>("./logfile/test.log");
//     yimingcode::LogSink::ptr roll_lsp = yimingcode::SinkFactory::create<yimingcode::RollBySizeSink>("./logfile/roll-",  1024 * 1024  );
//     std::vector<yimingcode::LogSink::ptr> sinks = {stdout_lsp, file_lsp, roll_lsp};
//    yimingcode::Logger::ptr logger(new yimingcode::SyncLogger(logger_name, limit, fmt, sinks));

    std::unique_ptr<yimingcode::LoggerBuilder> builder(new yimingcode::GlobalLoggerBuilder());
    builder->buildLoggerLevel(yimingcode::LogLevel::value::WARN);
    builder->buildLoggerName("async_logger");
    builder->buildFormatter("[%c][%f%l][%p]%m%n");
    builder->buildLoggerType(yimingcode::LoggerType::LOGGER_ASYNC);
    // builder->buildEnableUnSafeAsync();
    builder->buildSink<yimingcode::FileSink>("./logfile/async.log");
    builder->buildSink<yimingcode::StdoutSink>();
    builder->buildSink<yimingcode::RollBySizeSink>("./logfile/roll-async-by-size.log",1024*1024);
    builder->build();
    
        test_log("async_logger");
        return 0;

    
    //  std::cout<<yimingcode::LogLevel::toString(yimingcode::LogLevel::value::DEBUG);
    //  std::cout<<yimingcode::LogLevel::toString(yimingcode::LogLevel::value::INFO);
    //  std::cout<<yimingcode::LogLevel::toString(yimingcode::LogLevel::value::WARN);
    //  std::cout<<yimingcode::LogLevel::toString(yimingcode::LogLevel::value::ERROR);
    //  std::cout<<yimingcode::LogLevel::toString(yimingcode::LogLevel::value::FATAL);
    //  std::cout<<yimingcode::LogLevel::toString(yimingcode::LogLevel::value::OFF);
     
    // yimingcode::LogMsg msg(yimingcode::LogLevel::value::INFO, 53, "main.c","root","格式化功能测试");
    // yimingcode::Formatter fmt;
    // std::string str= fmt.format(msg);
    // // yimingcode::LogSink::ptr stdout_lsp =yimingcode::SinkFactory::create<yimingcode::StdoutSink>();

    // // yimingcode::LogSink::ptr file_lsp = yimingcode::SinkFactory::create<yimingcode::FileSink>("./logfile/test.log");
    // // yimingcode::LogSink::ptr roll_lsp = yimingcode::SinkFactory::create<yimingcode::RollBySizeSink>("./logfile/roll-",  1024 * 1024  );
    // yimingcode::LogSink::ptr time_lsp = yimingcode::SinkFactory::create<RollByTimeSink>("./logfile/roll-",TimeGap::GAP_SECOND);
    
    // time_t old = yimingcode::util::Date::now();

    // while(yimingcode::util::Date::now() < old + 5 )
    // {
    //     time_lsp->log(str.c_str(),str.size());
    // }


    // stdout_lsp->log(str.c_str(),str.size());
    // file_lsp->log(str.c_str(),str.size());
    // size_t cursize =0;
    // size_t count =0;
    // while(cursize<1024*1024)
    // {
    //     std::string tmp=str + std::to_string(count++);
    //     roll_lsp->log(tmp.c_str(),tmp.size());
    //     cursize+=tmp.size(); 
    // }
    // std::cout<< str<<std::endl;


    // // 创建测试文件
    // system("mkdir -p ./logfile");
    // std::ofstream create("./logfile/test.log", std::ios::binary);
    // create.write("测试内容\n", 12);
    // create.close();

    // //读取文件数据，一点一点写入缓冲区，最终将缓冲区写入文件，判断生成的文件是否与原文件一致
    // std::ifstream ifs("./logfile/test.log", std::ios::binary);
    // if(ifs.is_open()==false)
    // {
    //     std::cerr<<"文件打开失败"<<std::endl;
    //     return -1;
    // }
    // ifs.seekg(0,std::ios::end);//读写指针移动到文件末尾
    // size_t fsize = ifs.tellg();//获取文件大小
    // ifs.seekg(0,std::ios::beg);//读写指针移动到文件开头
    // std::string body;
    // body.resize(fsize);
    // ifs.read(&body[0],fsize);
    // if(ifs.good()==false)
    // {
    //     std::cerr<<"文件读取失败"<<std::endl;
    //     return -1;
    // }
    // ifs.close();

    // yimingcode::Buffer buffer;
    // for(int i=0;i<body.size();i++)
    // {
    //     buffer.push(&body[i],1);
    // }
    // std::ofstream ofs("./logfile/tmp.log", std::ios::binary);
    // ofs.write(buffer.begin(), buffer.readAbleSize());
    // ofs.close();
    // return 0;
    
}
