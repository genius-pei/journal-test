#ifndef YIMINGCODE_FORMAT_H__
#define YIMINGCODE_FORMAT_H__

#include "level.hpp"
#include <ctime>
#include "message.hpp"
#include <vector>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace yimingcode
{
    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem() = default;
        virtual void format(std::ostream &out, const LogMsg &msg) const = 0;
    };

    class MsgFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << msg._payload;
        }
    };

    class LevelFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << LogLevel::toString(msg._level);
        }
    };

    class TimeFormatItem : public FormatItem {
    public:
        TimeFormatItem(const std::string &format = "%H:%M:%S")
            : _time_fmt(format.empty() ? "%H:%M:%S" : format) {}

        void format(std::ostream& out, const LogMsg& msg) const override {
            struct tm t;
            localtime_r(&msg._ctime, &t);
            char tmp[32] = {0};
            strftime(tmp, sizeof(tmp), _time_fmt.c_str(), &t);
            out << tmp;
        }
    private:
        std::string _time_fmt;
    };

    class FileFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << msg._file;
        }
    };

    class LineFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << msg._line;
        }
    };

    class ThreadFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << msg._tid;
        }
    };

    class LoggerFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << msg._logger;
        }
    };

    class TabFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << '\t';
        }
    };

    class NLineFormatItem : public FormatItem {
    public:
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << '\n';
        }
    };

    class OtherFormatItem : public FormatItem {
    public:
        OtherFormatItem(const std::string &str) : _str(str) {}
        void format(std::ostream& out, const LogMsg& msg) const override {
            out << _str;
        }
    private:
        std::string _str;
    };

    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n")
            : _pattern(pattern)
        {
            if (!parsePattern()) {
                throw std::invalid_argument("Invalid format pattern: " + pattern);
            }
        }

        void format(std::ostream &out, const LogMsg &msg) const
        {
            for (auto &item : _items) {
                item->format(out, msg);
            }
        }

        std::string format(const LogMsg &msg) const
        {
            std::stringstream ss;
            format(ss, msg);
            return ss.str();
        }

        bool parsePattern()
        {
            std::vector<std::pair<std::string, std::string>> fmt_order;
            size_t pos = 0;

            while (pos < _pattern.size()) {
                if (_pattern[pos] != '%') {
                    std::string literal;
                    while (pos < _pattern.size() && _pattern[pos] != '%') {
                        literal.push_back(_pattern[pos++]);
                    }
                    if (!literal.empty()) {
                        fmt_order.emplace_back("", literal);
                    }
                    continue;
                }

                if (pos + 1 < _pattern.size() && _pattern[pos + 1] == '%') {
                    fmt_order.emplace_back("", "%");
                    pos += 2;
                    continue;
                }

                pos++;
                if (pos >= _pattern.size()) {
                    return false;
                }

                std::string key(1, _pattern[pos]);
                pos++;

                std::string val;
                if (pos < _pattern.size() && _pattern[pos] == '{') {
                    pos++;
                    while (pos < _pattern.size() && _pattern[pos] != '}') {
                        val.push_back(_pattern[pos++]);
                    }
                    if (pos >= _pattern.size()) {
                        return false;
                    }
                    pos++;
                }

                fmt_order.emplace_back(key, val);
            }

            for (auto &it : fmt_order) {
                auto item = createItem(it.first, it.second);
                if (!item) return false;
                _items.push_back(item);
            }
            return true;
        }

        FormatItem::ptr createItem(const std::string &key, const std::string &val)
        {
            if (key == "d") {
                std::string fmt = val.empty() ? "%H:%M:%S" : val;
                return std::make_shared<TimeFormatItem>(fmt);
            }
            if (key == "t") return std::make_shared<ThreadFormatItem>();
            if (key == "c") return std::make_shared<LoggerFormatItem>();
            if (key == "f") return std::make_shared<FileFormatItem>();
            if (key == "l") return std::make_shared<LineFormatItem>();
            if (key == "p") return std::make_shared<LevelFormatItem>();
            if (key == "T") return std::make_shared<TabFormatItem>();
            if (key == "m") return std::make_shared<MsgFormatItem>();
            if (key == "n") return std::make_shared<NLineFormatItem>();
            if (key == "") return std::make_shared<OtherFormatItem>(val);

            return FormatItem::ptr();
        }

    private:
        std::string _pattern;
        std::vector<FormatItem::ptr> _items;
    };
}

#endif