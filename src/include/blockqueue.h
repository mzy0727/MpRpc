#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

// 异步写日志
template<typename T>
class BlockQueue{
public:
    // 多个worker线程都会写日志queue
    void Push(const T &data){
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data); 
        m_conditionvariable.notify_one();
        //std::cout<<data<<std::endl;
    }
    // 只有一个线程读日志queue，写入日志文件
    T Pop(){
        std::unique_lock<std::mutex> lock(m_mutex);
        std::cout<<m_queue.size()<<std::endl;
        while (m_queue.empty())
        {
            // 日志队列为空，线程进入wait状态
            m_conditionvariable.wait(lock);
        }
        
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex  m_mutex;
    std::condition_variable m_conditionvariable;

};