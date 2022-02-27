#idndef __WEBSERVER_LOG_H__
#DEFINE __WEBSERVER_LOG_H_

#include <iostream>
#include <string>
#include <list>
#include <memory>


namespace webserver{

// 日志事件
class LogEvent{   // 使每次输出logger变为一个event
private:
    const char* m_fileName = nullptr;   //文件名
    int32_t m_line = 0;       //行号
    uint32_t m_elapse  = 0;   //程序从启动开始到现在的毫秒数
    uint32_t m_threadId = 0;  //线程编号
    uint32_t m_fiberId = 0;  //协程编号
    uint64_t m_time;        //时间戳
    std::string m_content;   //消息
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent();
};

// 日志级别
class LogLevel{
private:
public:
    enum Level {
        DEBUG = 1;
        INFO = 2;
        WARN = 3;
        ERROE = 4;
        FATAL = 5;
    };
};

//日志格式器, 格式化日志
class LogFormatter{
private:
    std::string m_pattern;  // 日志格式模板
    std::vector<FormatItem::ptr> m_items; //日志解析后的格式
    bool m_error = false;

    // 日志解析的子模块
    /*
     * 日志内容项格式化
     */
    class FormatItem{
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem() {}; //析构函数采用虚函数，防止内存泄漏，比如在基类中申请内存，那么虚构函数可以释放 
        /*
         * 格式化日至流
         * os 日志输出流
         * logger 日志器
         * level 日志等级
         * event 日志事件
         * */
        // 纯虚函数，给个抽象类，提醒必须派生
        virtual void format(std::ostream os, std::shared_ptr<Logger> logger, LogLevel::Level level,  LogEvent::ptr event) = 0;
    };


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
    LogFormatter(const std::string& pattern);
    /*
     * 返回格式化日志文本
     * logger 日志器
     * level 级别
     * event 事件
     * */
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);    //把event存为一个string，给Appender输出
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    /*
     * 初始化解析日志模板
     * */
    void init();

    const std::string getForamtter() const {return m_pattern;}
    
};


//日志输出的地方
class LogAppender{
private:
    LogLevel::Level m_level;   // Appender针对哪些等级的日志
    LogFormatter::ptr m_formatter;  
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){}

    void log(LogLevel::Level level, LogEvent::ptr event);

    void setFormatter(LogFormatter::ptr val){
        m_formatter = val;
    }

    LogFormatter::ptr getFormatter() const{
        return m_formatter;
    }
};

//日志器
class Logger{
private:
    LogLevel::Level level;   //定义日志器的级别,满足这个级别的才会被记录
    std::string m_name;      //日志名称
    std::list<LogAppender::ptr> m_appenders;       // Appender集合
public: 
    typedef std::shared_pre<Logger> ptr;
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level val){ m_level = val; }
};


// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(LogLevel::Level level, LogEvent::ptr event) override;
};


// 输出到文件的Appender
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    void log(LogLevel::Level level, LogEvent::ptr event)
    
    // 判断文件是否打开，已经打开则关闭重新打开,成功返回true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
};

}
