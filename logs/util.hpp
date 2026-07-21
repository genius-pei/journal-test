#ifndef __M_UTIL_H__
#define __M_UTIL_H__



#include<iostream>
#include<ctime>
#include<unistd.h>
#include<sys/stat.h>
#include<string>

namespace yimingcode{
   namespace util {

class Date {
public:
    static size_t now() { 
        return (size_t)time(nullptr); 
    }
};

class file {
public:
    static bool exists(const std::string &name) {
        struct stat st;
        return stat(name.c_str(), &st) == 0;
    }
    
    static std::string path(const std::string &name) {
        if (name.empty()) return ".";
        size_t pos = name.find_last_of("/\\");
        if (pos == std::string::npos) return ".";
        return name.substr(0, pos + 1);
    }
    
    static void create_directory(const std::string &path) { 
        if (path.empty()) return;
        if (exists(path)) return;
        
        size_t pos = 0;
        size_t idx = 0;
        
        // 处理绝对路径（以 / 或 \ 开头）
        if (path.size() > 0 && (path[0] == '/' || path[0] == '\\')) {
            idx = 1;
        }
        
        while (idx < path.size()) {
            pos = path.find_first_of("/\\", idx);
            
            if (pos == std::string::npos) {
                // 创建最后一级目录
                std::string last_dir = path;
                if (!exists(last_dir)) {
                    mkdir(last_dir.c_str(), 0755);
                }
                return;
            }
            
            // 跳过连续的路径分隔符
            if (pos == idx) {
                idx = pos + 1;
                continue;
            }
            
            std::string subdir = path.substr(0, pos);
            if (subdir == "." || subdir == "..") {
                idx = pos + 1;
                continue;
            }
            
            if (!exists(subdir)) {
                mkdir(subdir.c_str(), 0755);
            }
            
            idx = pos + 1;
        }
    }
};
}
}


#endif