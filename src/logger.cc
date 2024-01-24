#include "logger.h"
#include <time.h>
#include <cstring>
#include <iostream>
#include <stdarg.h>
#include <unistd.h>

bool Logger::init(char* file_name,int max_file_lines){
    strcpy(dir_name,file_name);
    m_file_lines = max_file_lines;

    // 获取当前时间
    time_t t = time(nullptr);
    tm *sys_time = localtime(&t);
    //构建文件名，在该文件中写入当天的日志信息
    sprintf(log_name,"%s%d_%02d_%02d_log.txt",dir_name,sys_time->tm_year + 1900,sys_time->tm_mon+1,sys_time->tm_mday);
    m_today = sys_time->tm_mday;
    
    m_fp = fopen(log_name, "a+");
    if (m_fp == NULL)
    {
       // fclose(m_fp);
        return false;
    }
   // fclose(m_fp);
    std::cout<<log_name<<std::endl;
    return true;
}
Logger::Logger(){
    //启动专门的写日志线程
    std::thread writeLogThread([&](){
        for(;;){
            //出队列，获取日志队列头中的信息
            std::string msg = m_queue.Pop();
            //将日志队列头中的信息写入到日志文件中
            fputs(msg.c_str(), m_fp);
            //fclose(m_fp);
           sleep(1);
        }   
    });
    writeLogThread.detach();
}

void Logger::write_log(int level, const char *format, ...){
    time_t t = time(nullptr);
    tm *sys_tm = localtime(&t);

    char s[16] = {0};
    switch (level){
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[erro]:");
            break;
        default:
            strcpy(s, "[info]:");
            break;
    }
    
    //写入一个log，对m_count++, max_file_lines最大行数
    m_count++;
   
    if (m_today != sys_tm->tm_mday || m_count % m_file_lines == 0) //everyday log
    {
        
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};
       
        snprintf(tail, 16, "%d_%02d_%02d_", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday);
       
        if (m_today != sys_tm->tm_mday)
        {
            snprintf(new_log, 255, "%s%s_log.txt", dir_name, tail);
            m_today = sys_tm->tm_mday;
            m_count = 0;
        }
        else
        {
            snprintf(new_log, 255, "%s%s%lld_log.txt", dir_name, tail, m_count / m_file_lines);
        }
        m_fp = fopen(new_log, "a+");

        
    }
    va_list valst;
    va_start(valst, format);
    //写入的具体时间内容格式
    int n = snprintf(m_buf, 100,"%d-%02d-%02d %02d:%02d:%02d %s ",
                    sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday,
                    sys_tm->tm_hour, sys_tm->tm_min, sys_tm->tm_sec, s);
    int m = vsnprintf(m_buf + n, 1024, format, valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    //std::cout<<""<<format<<std::endl;
   // std::cout<<m_buf<<std::endl;
    m_queue.Push(m_buf);
    
}

void Logger::flush()
{
    //强制刷新写入流缓冲区
    fflush(m_fp);
}