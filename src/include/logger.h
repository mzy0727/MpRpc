#pragma once
#include "blockqueue.h"
#include <string>
class Logger{

public:
    static Logger* getInstance(){
        static Logger logger;
        return &logger;
    }
    bool init(char* file_name,int max_file_lines);
    void write_log(int level, const char *format, ...);
    void flush();
private:
    Logger();
    Logger(const Logger &) =delete;
    Logger(const Logger &&) =delete;
    Logger& operator=(const Logger&) =delete;

    char dir_name[256];   // 路径名
    char log_name[256];   // 文件名
    int m_file_lines;     // 文件最大行数
    int m_count;          // 当前行数
    int m_today;          // 当前日期
    FILE *m_fp;
    char m_buf[1024];
    BlockQueue<std::string> m_queue;    // 日志缓冲队列

};


#define LOG_DEBUG(format, ...)  { Logger::getInstance()->write_log(0, format, ##__VA_ARGS__); Logger::getInstance()->flush();}
#define LOG_INFO(format, ...)   { Logger::getInstance()->write_log(1, format, ##__VA_ARGS__); Logger::getInstance()->flush();}
#define LOG_WARN(format, ...)   { Logger::getInstance()->write_log(2, format, ##__VA_ARGS__); Logger::getInstance()->flush();}
#define LOG_ERROR(format, ...)  { Logger::getInstance()->write_log(3, format, ##__VA_ARGS__); Logger::getInstance()->flush();}
