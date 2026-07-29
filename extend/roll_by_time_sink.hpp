#ifndef YIMINGCODE_ROLL_BY_TIME_SINK_H__
#define YIMINGCODE_ROLL_BY_TIME_SINK_H__

#include "../logs/sink.hpp"
#include <sstream>
#include <cassert>

enum class TimeGap
{
    GAP_SECOND,
    GAP_MINUTE,
    GAP_HOUR,
    GAP_DAY,
};

class RollByTimeSink : public yimingcode::LogSink {
public:
    RollByTimeSink(const std::string &basename, TimeGap gap_type)
        : _basename(basename)
    {
        switch (gap_type) {
            case TimeGap::GAP_SECOND: _gap_size = 1; break;
            case TimeGap::GAP_MINUTE: _gap_size = 60; break;
            case TimeGap::GAP_HOUR:   _gap_size = 3600; break;
            case TimeGap::GAP_DAY:    _gap_size = 3600 * 24; break;
        }
        _cur_gap = (_gap_size == 1) ? 0 : yimingcode::util::Date::now() % _gap_size;
        std::string filename = createNewFile();
        yimingcode::util::file::create_directory(yimingcode::util::file::path(filename));
        _ofs.open(filename, std::ios::binary | std::ios::app);
        assert(_ofs.is_open());
    }

    void log(const char* data, size_t len) {
        time_t cur = yimingcode::util::Date::now();
        size_t cur_slot = (cur % _gap_size);
        if (cur_slot != _cur_gap) {
            _cur_gap = cur_slot;
            _ofs.close();
            std::string filename = createNewFile();
            _ofs.open(filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }
        _ofs.write(data, len);
        assert(_ofs.good());
    }

    void flush() override {
        _ofs.flush();
    }

private:
    std::string createNewFile() {
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
    size_t _cur_gap;
    size_t _gap_size;
};

#endif
