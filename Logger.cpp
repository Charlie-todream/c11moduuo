#include "Logger.h"
#include "Timestamp.h"

#include <iostream>

// 获取日志实例对象
Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

// 设置日志 级别
void Logger::setLogLevel(int level)
{
    logLevel_ = level;
}

// 写日志 [日志级别] time : msg

void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[EEEOR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;

    default:
        break;
    }

    // 打印时间和msg
    // std::cout << Timestamp::now.to
}
