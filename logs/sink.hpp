#ifndef __M_SINK_H__
#define __M_SINK_H__
//日志落地模块的实现

#include"util.hpp"
#include<memory>
#include<fstream>
#include<cassert>
#include<sstream>
namespace yimingcode
{
    class LogSink
    {
        public:
        using ptr=std::shared_ptr<LogSink>;
        LogSink()
        {

        }
        virtual ~LogSink()
        {

        }
        virtual void log(const char* data,size_t len)=0;

    };
    //落地方向：标准输出
    class StdoutSink : public LogSink
    {
        public:
        
        //将日志消息写入到标准输出
            void log(const char* data,size_t len)
            {
                std::cout.write(data,len);//从 data位置开始写入len长度的数据
            }
            
    };
    //落地方向：指定文件
   class FileSink : public LogSink {
public:
    // 构造时传入文件名，打开文件，将操作句柄管理
    FileSink(const std::string &pathname)
    :_pathname(pathname)
    {
        //1.创建日志文件所在的目录
        util::file::create_directory(util::file::path(pathname));
        //2.创建并打开日志文件
        _ofs.open(_pathname,std::ios::binary|std::ios::app);
        assert(_ofs.is_open());
        
    }
    
    
    void log(const char* data,size_t len) override {
        _ofs.write(data,len);
        assert(_ofs.good());
    }
   
private:
    std::string _pathname;
    std::ofstream _ofs;
};

    //落地方向：滚动文件
    class RollBySizeSink : public LogSink
{
public:
    RollBySizeSink(const std::string &basename, size_t max_size)
    :_basename(basename)
    ,_max_fsize(max_size)
    ,_cur_fsize(0)
    ,_name_count(0)
    {
        std::string pathname = createNewFile();
        util::file::create_directory(util::file::path(pathname));
        _ofs.open(pathname, std::ios::binary | std::ios::app);
        assert(_ofs.is_open());
        
        //  获取当前文件大小（如果文件已存在）
        _ofs.seekp(0, std::ios::end);
        _cur_fsize = _ofs.tellp();
    }
    
    void log(const char* data, size_t len)
    {
        //  用 + len 判断，防止刚好超过
        if (_cur_fsize + len > _max_fsize)
        {
            std::string pathname = createNewFile();
            _ofs.close();
            _ofs.open(pathname, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
            _cur_fsize = 0;
        }
        _ofs.write(data, len);
        assert(_ofs.good());
        _cur_fsize += len;
    }
    
private:
    std::string createNewFile()
    {
        time_t t = util::Date::now();
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
                 << "-"
                 << _name_count++
                 << ".log";
        return filename.str();
    }

private:
    std::string _basename;
    std::ofstream _ofs;
    size_t _max_fsize;
    size_t _cur_fsize;
    size_t _name_count;
};

    
    class SinkFactory
    {
        public:
        template<typename SinkType,typename ...Args>
         static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };


}

#endif