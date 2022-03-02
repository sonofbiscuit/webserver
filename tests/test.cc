#include <iostream>
#include "../webserver/log.h"

int main(int argc, char** argv){   //  or : (int argc, int* argv[])
    // argc 是命令行的总参数个数
    // argv** 由argc个参数，其中第0个参数是程序全名，命令行后面跟的用户输入的参数
    // 添加新的appender
    webserver::Logger::ptr logger(new webserver::Logger);
    logger->addAppender(webserver::Logger->addAppender(new webserver::StdoutAppender));
    
    // 添加新的event
    webserver::LogEvent::ptr event(new webserver::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));

    logger->log(LogLevel::Level::DEBUG, event);
    
    std::cout << "my log" << std::endl;

    return 0;
}
