//实现异步日志缓冲区
#ifndef __M_BUF_H__
#define __M_BUF_H__
#include"util.hpp"
#include<vector>
#include<assert.h>
#include<algorithm>  // 添加 algorithm 头文件


namespace yimingcode
{
    #define DEFAULT_BUFFER_SIZE (1024*1024*1)      // 1MB
    #define THRESHOLD_BUFFER_SIZE (1024*1024*8)    // 8MB
    #define INCREMENT_BUFFER_SIZE (1024*1024*1)    // 1MB
    
    class Buffer
    {
    public:
        Buffer()
            :_buffer(DEFAULT_BUFFER_SIZE)
            ,_reader_idx(0)
            ,_writer_idx(0)
        {
        }
        
        //将日志数据写入缓冲区
        void push(const char* data, size_t len)
        {
            if (len == 0) return;
            
            // 确保有足够的空间
            ensureEnoughSpace(len);
            
            // 将数据拷贝进缓冲区
            std::copy(data, data + len, &_buffer[_writer_idx]);
            
            // 移动写指针
            _writer_idx += len;
        }
        
        //返回当前可读数据的起始位置
        const char* begin() const
        {
            return &_buffer[_reader_idx];
        }
        
        //返回当前可读数据的长度
        size_t readAbleSize() const
        {
            return (_writer_idx - _reader_idx);
        }
        
        //返回当前可写数据的长度
        size_t writeAbleSize() const
        {
            return (_buffer.size() - _writer_idx);
        }
        
        //对读写指针进行向后偏移操作
        void moveWriter(size_t len)
        {
            assert(len <= writeAbleSize());
            _writer_idx += len;
        }
        
        void moveReader(size_t len)
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }
        
        //重置缓冲区
        void reset()
        {
            _reader_idx = 0;
            _writer_idx = 0;
        }
        
        //交换缓冲区
        void swap(Buffer& buffer)
        {
            _buffer.swap(buffer._buffer);
            std::swap(_reader_idx, buffer._reader_idx);
            std::swap(_writer_idx, buffer._writer_idx);
        }
        
        //判断缓冲区是否为空
        bool empty() const
        {
            return (_reader_idx == _writer_idx);
        }
        
        // 获取缓冲区容量
        size_t capacity() const
        {
            return _buffer.size();
        }
        
    private:
        // 确保有足够的空间写入 len 字节的数据
        void ensureEnoughSpace(size_t len)
        {
            if (len <= writeAbleSize()) 
                return; // 空间足够，不需要扩容
            
            // 计算需要的最小新容量
            size_t need_size = _writer_idx + len;
            size_t new_size = _buffer.size();
            
            // 动态扩容策略
            while (new_size < need_size) {
                if (new_size < THRESHOLD_BUFFER_SIZE) {
                    // 小于阈值，直接扩容为原来的两倍
                    new_size = new_size * 2;
                } else {
                    // 大于阈值，每次增加固定大小
                    new_size = new_size + INCREMENT_BUFFER_SIZE;
                }
            }
            
            // 执行扩容
            _buffer.resize(new_size);
        }
        
    private:
        std::vector<char> _buffer;
        size_t _reader_idx;  // 当前可读数据的指针
        size_t _writer_idx;  // 当前可写数据的指针
    };
    
}
#endif