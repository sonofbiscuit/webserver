#include "log.h"
#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>


namespace webserver {
    // 宏定义函数，在编译阶段执行，不影响程序运行时间。和内联函数类似
    const char* LogLevel::ToString(LogLevel::Level level){
        switch(level){
        #define XX(name) \
            case LogLevel::name: \
                return #name;    \
                break;
            //   “\” 是换行继续的意思
            // #name 表示使用传入的“参数名称”字符串，注意不是name值本身
            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
        # undef XX
            default :
                return "UNKNOWN";
        }
        return "UNKNOWN";
    }

    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << event->getElapse();
        }
    };

    // 日志器的名称，formatter拿不到日志器名称，直接把logger往下传
    class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << logger->getName();
        }
    };

    // 线程id
    class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << event->getThreadId();
        }
    };

    // 协程id
    class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
        FiberIdFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << event->getFiberId();
        }
    };

    // 时间
    class DataTimeFormatItem : public LogFormatter::FormatItem {
    public:
        DataTimeFormatItem(const std::string format = "%Y-%m-%d %H:%M:%S")
                :m_format(format){
        }
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << event->getThreadId();
        }
    private:
        std::string m_format;  // 时间的格式
    };

    // 文件名
    class FileNameFormatItem : public LogFormatter::FormatItem {
    public:
        FileNameFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << event->getFile();
        }
    };

    // 行号
    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << event->getLine();
        }
    };

    // 换行符
    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string& str = ""){}
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << std::endl;
        }
    };

    //string 格式
    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std::string& str)
            : FormatItem(str), m_string(str){
        }
        void format(std::shared_ptr<Logger> logger, std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override{
            os << m_string;
        }

    private:
        std::string m_string;
    };

    Logger::Logger(const std::string &name)
            : m_name(name)
            , m_level(LogLevel::DEBUG) () {
        // 初始化个formatter， 比如有时候appender不需要formatter，直接使用logformatter
        m_formatter.reset(new LogFormatter("%d [%p] %f %l %m %n"));
    }

    void Logger::addAppender(LogAppender::ptr appender) {
        if(!appender->getFormatter){  // 如果没有formatter，那么设置为默认
            appender->setFormatter(m_formatter);
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender) {
        //  遍历的方式删除
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            if (*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {  //日志器的记录级别大于当前的事件级别，才会记录
            auto self = shared_from_this(); //返回一个当前类的std::shared_ptr
            for (auto &i: m_appenders) {
                i->log(self, level, event);  // param (logger, level, event)
            }
        }
    }

    void Logger::debug(LogEvent::ptr event) {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event) {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::FATAL, event);
    }

    // 输出到文件的日志
    FileLogAppender::FileLogAppender(const std::string &filename)
            : m_filename(filename) {   // 初始化日志事件的name
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            m_filestream << m_formatter->format(logger, level, event); // 存为一个string，后续交给appender
        }
    }


    bool FileLogAppender::reopen() { //private已经给了个fstream,直接判断即可
        if (m_filestream) { //如果打开
            m_filestream.close();
        }
        m_filestream.open(m_filename);
    }

    // 输出到控制台的appender
    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        if(level >= m_level){
            std::cout<< m_formatter->format(logger, level, event);
        }
    }


    LogFormatter::LogFormatter(const std::string &pattern)
            : m_pattern(pattern) {
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
        // 遍历获取pattern
        std::stringstream ss;
        for (auto &a: m_items) {
            a->format(logger, ss, level, event);
        }
        return ss.str(); // return content
    }


// %xxx  %xxx{xxx} %%   类型  类型{格式}  需要输出%(即转义)   其余为正常文本格式
    void LogFormatter::inits() {
        // 解析日志
        // str, format, type   string，格式，类别
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr; // 存取正常的string格式内容，   xxxxx这种   normalstring
        for (size_t i = 0; i < m_pattern.size(); ++i) {
            /*
             * 外层循环遍历整个字符串，直至遇到%
             * 内层循环从%下一个字符开始遍历，同时标记为状态0
             * 如果在状态0的情况下遇到{， 标记为状态1 ， 获取%到{之间的子串
             * 如果在状态1的情况下遇到}， 标记为状态2，获取{ 到 }之间的子串
             * 剩下的就是对获取的子串进行具体操作
             * */
            if (m_pattern[i] != '%') {  //有字符
                nstr.append(1, m_pattern[i]);
                continue;
            }

            // 不为字符且下一个也为%
            if ((i + 1) < m_pattern.size()) { // 判断是否转义
                if (m_pattern[i + 1] == '%') {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int formatter_status = 0; // 初始化formatter的状态, 0表示不解析，1表示解析
            size_t formatter_begin = 0; // 记录'{'的位置， n-i-1表示的是括号内的字符长度
            /*
             * formatter_status 的状态为0， 那么说明存入的就是string类型
             * */
            std::string str;
            std::string fmt;
            // 根据默认格式来解析
            while (n < m_pattern.size()) {
                if (isspace(m_pattern[n])) {  // 为空格
                    break;
                }

                if (formatter_status == 0) {
                    if (m_pattern[n] == '{') {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        formatter_status = 1;   // 开始解析
                        formatter_begin = n;
                        ++n;
                        continue;
                    }
                }
                if (formatter_status == 1) {  //要求解析
                    if (m_pattern[n] == '}') {  // m_pattern已经结束
                        fmt = m_pattern.substr(formatter_begin + 1,
                                               n - formatter_begin - 1); //获取{}内的所有内容，formatter_begin记录的是‘{’位置
                        formatter_status = 2;
                        ++n;
                        break;
                    }
                }
                /*++n;
                if (n == m_pattern.size()) {
                    if (str.empty()) { //str为空
                        str = m_pattern.substr(i + 1);  // m_pattern为 %xxxxx 全是字符
                    }
                }*/
            }

            // m_pattern 遍历完了，且未遇到括号{}，遇到'{' status=1，右括号'}'status=2
            if (formatter_status == 0) {
                if (!nstr.empty()) { // 非空
                    vec.emplace_back(std::make_tuple(nstr, "", 0));   // normalstring状态码记为0
                    nstr.clear();
                }
                //nstr为空
                str = m_pattern.substr(i + 1, n - i - 1);
                vec.emplace_back(std::make_tuple(str, fmt, 1));
                i = n;
            } else if (formatter_status == 1) {
                std::cout << "pattern_error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                m_error = true;
                vec.emplace_back(std::make_tuple("<<pattern error>>", fmt, 0));
            } else if (formatter_status == 2) {
                if (!nstr.empty()) { // 非空
                    vec.emplace_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }
                vec.emplace_back(std::make_tuple(str, fmt, 1));
                i = n;
            }
        }
        if (!nstr.empty()) {
            vec.emplace_back(std::make_tuple(nstr, "", 0));
        }

        // 给个映射关系,string -> function, function的返回值是 FormatItem::ptr 以对应不同class
        /*
         * %m -- 消息体
         * %p -- level
         * %r -- 启动后运行的时间
         * %c -- 日志器logger名称
         * %t -- 线程id
         * %n -- 回车换行
         * %d -- 时间
         * %f -- 文件名
         * %l -- 行号
         * */
        static std::map <std::string, std::function<FormatItem::ptr(const std::string& fmt)>> s_format_items = {
            #define XX(str,C) \
                {#str, [](const std::string& fmt) {return FormatItem::ptr(new C(fmt));} }

                XX(m, MessageFormatItem),
                XX(p, LevelFormatItem),
                XX(r, ElapseFormatItem),
                XX(c, NameFormatItem),
                XX(t, ThreadIdFormatItem),
                //  XX(m, FiberIdFormatItem),
                XX(n, NewLineFormatItem),
                XX(d, DataTimeFormatItem),
                XX(f, FileNameFormatItem),
                XX(l, LineFormatItem),
        };

        for(auto& i : vec){   // @param : str, 格式， 类别
            if(std::get<2>(i) == 0){   // normal string
                m_items.emplace_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }else{   //%xxx   和 %%xxx 转义
                auto it = std::get<1>(i);
                auto fd = s_format_items.find(it);
                if(fd != s_format_items.end()){  //找到对应格式
                    m_items.emplace_back(FormatItem::ptr(fd->second(std::get<1>(i))));
                    // fd->second 为 std::function<FormatItem::ptr(const std::string& fmt)>
                    // 传个参数  std::get<1>(i)  以获得对应格式
                }else{ //未知格式
                    m_items.emplace_back(FormatItem::ptr(new StringFormatItem("<< error format %" + std::get<1>(i) + ">>")));
                    m_error = true;
                }
            }

            // debug
            std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i) <<std::endl;
        }

    }

    const char *m_fileName = nullptr;   //文件名
    int32_t m_line = 0;       //行号
    uint32_t m_elapse = 0;   //程序从启动开始到现在的毫秒数
    uint32_t m_threadId = 0;  //线程编号
    uint32_t m_fiberId = 0;  //协程编号
    uint64_t m_time;        //时间戳
    std::string m_content;   //消息














}





















