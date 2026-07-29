#ifndef YIMINGCODE_SINK_H__
#define YIMINGCODE_SINK_H__

#include "util.hpp"
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace yimingcode
{
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        virtual ~LogSink() = default;
        virtual void log(const char* data, size_t len) = 0;
        virtual void flush() {}
    };

    class StdoutSink : public LogSink
    {
    public:
        void log(const char* data, size_t len) override
        {
            std::cout.write(data, len);
        }

        void flush() override
        {
            std::cout.flush();
        }
    };

    class FileSink : public LogSink
    {
    public:
        FileSink(const std::string &pathname)
            : _pathname(pathname)
        {
            util::file::create_directory(util::file::path(pathname));
            _ofs.open(_pathname, std::ios::binary | std::ios::app);
            if (!_ofs.is_open()) {
                throw std::runtime_error("Failed to open log file: " + _pathname);
            }
        }

        void log(const char* data, size_t len) override
        {
            _ofs.write(data, len);
            if (!_ofs.good()) {
                throw std::runtime_error("Write failed for log file: " + _pathname);
            }
        }

        void flush() override
        {
            _ofs.flush();
        }

    private:
        std::string _pathname;
        std::ofstream _ofs;
    };

    class RollBySizeSink : public LogSink
    {
    public:
        RollBySizeSink(const std::string &basename, size_t max_size)
            : _basename(basename)
            , _max_fsize(max_size)
            , _cur_fsize(0)
            , _name_count(0)
        {
            std::string pathname = createNewFile();
            util::file::create_directory(util::file::path(pathname));
            _ofs.open(pathname, std::ios::binary | std::ios::app);
            if (!_ofs.is_open()) {
                throw std::runtime_error("Failed to open roll log file: " + pathname);
            }
            _ofs.seekp(0, std::ios::end);
            _cur_fsize = static_cast<size_t>(_ofs.tellp());
        }

        void log(const char* data, size_t len) override
        {
            if (_cur_fsize + len > _max_fsize)
            {
                std::string pathname = createNewFile();
                _ofs.close();
                _ofs.open(pathname, std::ios::binary | std::ios::app);
                if (!_ofs.is_open()) {
                    throw std::runtime_error("Failed to open roll log file: " + pathname);
                }
                _cur_fsize = 0;
            }
            _ofs.write(data, len);
            if (!_ofs.good()) {
                throw std::runtime_error("Write failed for roll log file: " + _basename);
            }
            _cur_fsize += len;
        }

        void flush() override
        {
            _ofs.flush();
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
        template<typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
}

#endif