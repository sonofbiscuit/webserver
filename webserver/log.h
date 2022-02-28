#ifndef __WEBSERVER_LOG_H__
#define __WEBSERVER_LOG_H_

class ptr;

class ptr;

#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <tuple>
#include <stdarg.h>
#include <map>


namespace webserver {
    class Logger;  // Logger 定义在之后，再写个class方便传参

// 日志事件
    class LogEvent {   // 使每次输出logger变为一个event
    private:
        const char *m_fileName = nullptr;   //文件名
        int32_t m_line = 0;       //行号
        uint32_t m_elapse = 0;   //程序从启动开始到现在的毫秒数
        uint32_t m_threadId = 0;  //线程编号
        uint32_t m_fiberId = 0;  //协程编号
        uint64_t m_time;        //时间戳
        std::string m_content;   //消息
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();

        const char* getFile() const {return m_fileName;}
        int32_t getLine() const {return m_line;}
        uint32_t getElapse() const {return m_elapse;}
        uint32_t getThreadId() const {return m_threadId;}
        uint32_t getFiberId() const {return m_fiberId;}
        uint64_t getTime() const {return m_time;}
        const std::string getContent() const {return m_content;}
    };

// 日志级别
    class LogLevel {
    private:
    public:
        enum Level {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
        static const char* ToString(LogLevel::Level level);

    };

//日志格式器, 格式化日志
    class LogFormatter {
    public:
        // 日志解析的子模块
        /*
         * 日志内容项格式化
         */
        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            FormatItem(const std::string& fmt = "") {};
            virtual ~FormatItem() {}; //析构函数采用虚函数，防止内存泄漏，比如在基类中申请内存，那么虚构函数可以释放
            /*
             * 格式化日志流
             * os 日志输出流
             * logger 日志器
             * level 日志等级
             * event 日志事件
             * */
            // 纯虚函数，给个抽象类，提醒必须派生
            // 传logger进来，不然没法获取logger的private
            virtual void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    private:
        std::string m_pattern;  // 日志格式模板
        std::vector<FormatItem::ptr> m_items; //日志解析后的格式
        bool m_error = false;

    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        /*
         * 构造函数
         * @param[in]    pattern 格式模板
         *
         * %m 消息
         * %p 日志级别
         * %r 累计毫秒数
         * %c 日志名称
         * %t 线程id
         * %n 换行
         * %d 时间
         * %f 文件名称
         * %l 行号
         * %T 制表符
         * %F 协程号
         * %N 线程名称
         *
         * 默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
         * */
        LogFormatter(const std::string &pattern);

        /*
         * 返回格式化日志文本
         * logger 日志器
         * level 级别
         * event 事件
         * */
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);    //把event存为一个string，给Appender输出

        /*
         * 初始化解析日志模板
         * */
        void inits();
        const std::string getFormatter() const { return m_pattern; }
        // getFormatter的返回值不允许被更改
        // 第二个const使得该函数的权限为只读，即无法去改变成员变量的值
        // const对象只能调用const成员函数，非const对象均可调用
    };


//日志输出的地方
    class LogAppender {
    protected:
        LogLevel::Level m_level;   // Appender针对哪些等级的日志
        LogFormatter::ptr m_formatter;  // 日志格式器
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        virtual ~LogAppender() {}

        // 把logger传到appender，方便后续输出logger的名称，不然没法获取private
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val) {
            m_formatter = val;
        }

        LogFormatter::ptr getFormatter() const {
            return m_formatter;
        }
    };

//日志器
    class Logger : public std::enable_shared_from_this<Logger> {
    private:
        LogLevel::Level m_level;   //定义日志器的级别,满足这个级别的才会被记录
        std::string m_name;      //日志器logger名称
        std::list<LogAppender::ptr> m_appenders;       // Appender集合
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

        const std::string& getName() const { return m_name;}
    };


// 输出到控制台的Appender
    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    };


// 输出到文件的Appender
    class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

        // 判断文件是否打开，已经打开则关闭重新打开,成功返回true
        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}
#endif