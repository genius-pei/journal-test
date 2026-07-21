#ifndef __M_FMT_H__
#define __M_FMT_H__

#include"level.hpp"
#include<ctime>
#include "message.hpp"
#include<vector>
#include <memory>
#include<assert.h>
#include<sstream>

namespace yimingcode
{
  class FormatItem//抽象格式化子类基项
  {
    public:
    using ptr = std::shared_ptr<FormatItem>;
    virtual ~FormatItem() {}

    virtual void format(std::ostream &out, const LogMsg &msg) const =0;
  };
  
  class MsgFormatItem : public FormatItem {  //消息格式化子项
  public:
    MsgFormatItem(const std::string &str = ""){}
    
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << msg._payload;
    }
  };

  class LevelFormatItem : public FormatItem {  //等级格式化子项
  public:
    LevelFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << LogLevel::toString(msg._level);
    }
  };

  class TimeFormatItem : public FormatItem {  //时间格式化子项
  public:
    TimeFormatItem(const std::string &format = "%H:%M:%S")
        : _time_fmt(format) 
    {
        if (format.empty()) _time_fmt = "%H:%M:%S";
    }
    
    void format(std::ostream& out, const LogMsg& msg) const override {
        struct tm t;
        localtime_r(&msg._ctime,&t);
        char tmp[32] ={0};
        strftime(tmp,31,_time_fmt.c_str(),&t);
        out<<tmp;
    }
  private:
    std::string _time_fmt;
  };

  class FileFormatItem : public FormatItem {  //文件名格式化子项
  public:
    FileFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << msg._file;
    }
  };

  class LineFormatItem : public FormatItem {  //行号格式化子项
  public:
    LineFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << msg._line;
    }
  };

  class ThreadFormatItem : public FormatItem {  //线程ID格式化子项
  public:
    ThreadFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << msg._tid;
    }
  };

  class LoggerFormatItem : public FormatItem {  //日志器名称格式化子项
  public:
    LoggerFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << msg._logger;
    }
  };

  class TabFormatItem : public FormatItem {  //制表符
  public:
    TabFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << '\t';
    }
  };

  class NLineFormatItem : public FormatItem {  //换行
  public:
    NLineFormatItem(const std::string &str = ""){}
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << "\n";
    }
  };

  class OtherFormatItem : public FormatItem {  //普通字符串
  public:
    OtherFormatItem(const std::string &str)
        :_str(str)
    {
    }
    void format(std::ostream& out, const LogMsg& msg) const override {
        out << _str;
    }
  private:
    std::string _str;
  };

  /*
    %d 日期
    %t 线程id
    %c 日志器名称
    %l 原码行号
    %p 日志等级
    %T 制表符缩进
    %m 主体消息
    %n 换行
  */
  class Formatter
  {
  public:
    using ptr=std::shared_ptr<Formatter>;
    Formatter(const std::string &pattern ="[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n"):
      _pattern(pattern)
    {
        assert(parsePattern());
    }
    
    void format(std::ostream &out, const LogMsg &msg) const
    {
        for(auto &item :_items)
        {
            item->format(out,msg);
        }
    }
    
    std::string format(const LogMsg &msg) const
    {
        std::stringstream ss;
        format(ss,msg);
        return ss.str();
    }
    
    
    bool parsePattern()  // 对格式化规则字符进行解析
{
    std::vector<std::pair<std::string, std::string>> fmt_order;
    size_t pos = 0;
    
    while (pos < _pattern.size()) {
        // 情况1: 不是 % 开头，收集普通字符串
        if (_pattern[pos] != '%') {
            std::string literal;
            while (pos < _pattern.size() && _pattern[pos] != '%') {
                literal.push_back(_pattern[pos++]);
            }
            if (!literal.empty()) {
                fmt_order.push_back(std::make_pair("", literal));
            }
            continue;
        }
        
        // 情况2: 遇到 %，检查是否是 %%（转义为普通%）
        if (pos + 1 < _pattern.size() && _pattern[pos + 1] == '%') {
            fmt_order.push_back(std::make_pair("", "%"));
            pos += 2;
            continue;
        }
        
        // 情况3: 真正的格式字符 %d, %t 等
        pos++;  // 跳过 %
        if (pos >= _pattern.size()) {
            std::cout << "%之后没有对应的格式化字符!\n";
            return false;
        }
        
        std::string key(1, _pattern[pos]);
        pos++;  // 跳过格式字符
        
        // 检查是否有 { }
        std::string val;
        if (pos < _pattern.size() && _pattern[pos] == '{') {
            pos++;  // 跳过 {
            while (pos < _pattern.size() && _pattern[pos] != '}') {
                val.push_back(_pattern[pos++]);
            }
            if (pos >= _pattern.size()) {
                std::cout << "子规则{}匹配出错！\n";
                return false;
            }
            pos++;  // 跳过 }
        }
        
        fmt_order.push_back(std::make_pair(key, val));
    }
    
    // 创建格式化项
    for (auto &it : fmt_order) {
        _items.push_back(createItem(it.first, it.second));
    }
    return true;
}

 
    FormatItem::ptr createItem(const std::string &key, const std::string &val)
    {
        if(key == "d") {
            std::string fmt = val.empty() ? "%H:%M:%S" : val;
            return std::make_shared<TimeFormatItem>(fmt);
        }
        if(key == "t") {
            return std::make_shared<ThreadFormatItem>();
        }
        if(key == "c") {
            return std::make_shared<LoggerFormatItem>();
        }
        if(key == "f") {
            return std::make_shared<FileFormatItem>();
        }
        if(key == "l") {
            return std::make_shared<LineFormatItem>();
        }
        if(key == "p") {
            return std::make_shared<LevelFormatItem>();
        }
        if(key == "T") {
            return std::make_shared<TabFormatItem>();
        }
        if(key == "m") {
            return std::make_shared<MsgFormatItem>();
        }
        if(key == "n") {
            return std::make_shared<NLineFormatItem>();
        }
        
        if(key==""){
            return std::make_shared<OtherFormatItem>(val);
        }
        
   
        std::cout<<"没有对应的格式化字符:%"<<key<<std::endl;
            abort();
        
        return FormatItem::ptr();
    }
    
  private:
    std::string _pattern;
    std::vector<FormatItem::ptr> _items;
  };
}

#endif