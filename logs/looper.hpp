#ifndef __M_LOOPER_H__
#define __M_LOOPER_H__
#include"buffer.hpp"
#include<mutex>
#include<thread>
#include<condition_variable>
#include<functional>
#include<memory>
#include<atomic>

namespace yimingcode
{
    using Functor = std::function<void(Buffer &)>;
    enum class AsyncType
    {
        ASYNC_SAFE,//安全的异步日志
        ASYNC_UNSAFE//不考虑资源耗尽的情况，无限扩容
    };
    class ASyncLogLooper
    {
        public:
            using ptr = std::shared_ptr<ASyncLogLooper>;
            ASyncLogLooper()
            :_stop(false)
            ,_looper_type(AsyncType::ASYNC_SAFE)
            ,_thread(std::thread(&ASyncLogLooper::threadEntry, this))
            {
            }

            ASyncLogLooper(Functor cb, AsyncType type)
            :_callBack(cb)
            ,_stop(false)
            ,_looper_type(type)
            ,_thread(std::thread(&ASyncLogLooper::threadEntry, this))
            {
            }
            ~ASyncLogLooper()
            {
                stop();
            }
             void stop()
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _stop = true;
                    // 唤醒所有等待的线程
                    _cond_pro.notify_all();
                    _cond_con.notify_all();
                }
                
                //  等待线程结束
                if (_thread.joinable()) {
                    _thread.join();
                }
            }
            void push(const char* data, size_t len)
            {
                std::unique_lock<std::mutex> lock(_mutex);
                
                if (_looper_type == AsyncType::ASYNC_UNSAFE) {
                    // 不安全模式：直接写入，如果空间不足则扩容
                    _pro_buf.push(data, len);
                } else {
                    // 安全模式：如果空间不足则等待
                    // 注意：如果 len 大于缓冲区总大小，永远无法写入
                    if (len > _pro_buf.capacity()) {
                        // 1：直接扩容
                        _pro_buf.push(data, len);
                        // 2：丢弃日志并记录错误
                        // std::cerr << "Log message too large: " << len << " bytes" << std::endl;
                        // return;
                    } else {
                        // 等待直到有足够的空间
                        _cond_pro.wait(lock, [&](){ 
                            return _pro_buf.writeAbleSize() >= len; 
                        });
                        _pro_buf.push(data, len);
                    }
                }
                
                // 唤醒消费者线程
                _cond_con.notify_one();
            }
                        
        private:
        //线程入口函数--对消费缓冲区的数据进行处理，处理完毕后，初始化缓冲区，交换缓冲区
           void threadEntry()
        {
            while (true) {
                std::unique_lock<std::mutex> lock(_mutex);
                
                //  等待有数据或需要停止
                _cond_con.wait(lock, [&]() { 
                    return _stop || !_pro_buf.empty(); 
                });
                
                //  如果停止且没有数据，退出
                if (_stop && _pro_buf.empty()) {
                    break;
                }
                
                // 交换缓冲区
                _con_buf.swap(_pro_buf);
                
                //  唤醒生产者（安全模式）
                if (_looper_type == AsyncType::ASYNC_SAFE) {
                    _cond_pro.notify_all();
                }
                
                //  解锁后处理数据（避免长时间持有锁）
                lock.unlock();
                
                //  处理数据
                if (_callBack && _con_buf.readAbleSize() > 0) {
                    _callBack(_con_buf);
                }
                
                //  重置缓冲区
                _con_buf.reset();
                
                // 重新加锁，继续循环
                lock.lock();
            }
        }
        private:
            Functor _callBack;//回调函数
             
        private:
        AsyncType _looper_type;
        std::atomic<bool> _stop;
        Buffer _pro_buf;//生产者缓冲区
        Buffer _con_buf;//消费者缓冲区
        std::mutex _mutex;
        std::condition_variable  _cond_pro;
        std::condition_variable  _cond_con;
        std::thread _thread;//异步工作器对应的工作线程
    };
}




















#endif
