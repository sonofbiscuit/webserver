include "log.h"

namespace webserver{

Logger(const std::string& name = "root")
    :m_name(name){

}

void Logger::addAppender(LogAppender::ptr appender){
    m_appender.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
    //  遍历的方式删除
    for(auto it = m_appenders.begin();it != m_appenders.end();++it){
        if(*it == appender){
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){  //日志器的记录级别大于当前的事件级别，才会记录
        for(auto& i:m_appenders){
            i->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event){
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event){
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event){
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event){
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::FATAL, event);
}

// 输出到文件的日志
FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename){   // 初始化日志事件的name
}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        m_filestream << m_formatter->format(event); // 存为一个string，后续交给appender
    }
}


bool FileLogAppender::reopen(){ //private已经给了个fstream,直接判断即可
    if(m_filestream){ //如果打开
        m_filestream.close();
    }
    m_filestream.open(m_filename);
}


LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern){
}

LogFormatter::init(){

}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event){
    // 遍历获取pattern
    std::stringstream ss;
    for(auto& a : m_items){
        a->format(ss, logger, level, event);
    }
    return ss.str(); // return content
}

std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event){

}


// %xxx  %xxx{xxx} %%   类型  类型{格式}  需要输出%(即转义)   其余为正常文本格式
void LogFormatter::init(){
    // 解析日志
    // str, format, type   string，格式，类别
    std::vector<tuple<std::string, std::string, int>> vec;
    std::string nstr; //表示当前的string是什么
    for(int i = 0; i < m_pattern.size();++i){
        if(m_pattern[i] != %){  //有字符
            nstr.append(1, m_pattern[i]);
            continue;
        }
        
        // 不为字符且下一个也为%
        if((i+1) < m_pattern.size()){ // 判断是否转义
            if(m_pattern[i+1] == %){  
                nstr.append(1, m_pattern[i+1]);
                continue;
            }
        }

        size_t n= i + 1;
        int formatter_status = 0; // 初始化formatter的状态, 0表示不解析，1表示解析
        size_t formatter_begin = 0; // 记录'{'的位置， n-i-1表示的是括号内的字符长度

        std::string str;
        std::string fmt;
        // 根据默认格式来解析
        while(n < m_pattern.size()){
            if(isspace(m_pattern[n])){  // 为空格
                break;
            }

            // m_pattern[n]不为字母并且不为括号， 并且状态码为0, 类似于 xxxxx%   ???
            if((!isalpha(m_pattern[n]) && m_pattern[n]!='{' && m_pattern[n]!='}') && !formatter_status){
                str = m_patter.substr(i + 1,n - i - 1);
                break;
            }

            if(formatter_status == 0){
                if(m_pattern[n] == '{'){
                    str = m_pattern.substr(i+1, n-i);
                    formatter_status = 1;   // 开始解析
                    formatter_begin = n;
                    ++n;
                    continue;
                }
            }else if(formatter_status == 1){  //要求解析
                if(m_pattern[n] == '}'){  // m_pattern已经结束
                    fmt = m_appender.substr(formatter_begin, n - formatter_begin - 1); //获取{}内的所有内容，formatter_begin记录的是‘{’位置
                    formatter_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()){
                if(str.empty()){ //str为空
                    str = m_pattern.substr(i+1);  // m_pattern为 %xxxxx 全是字符
                }
            }
        }
        
        // m_pattern 遍历完了，且未遇到括号{}，遇到'{' status=1，右括号'}'status=2
        if(formatter_status == 0){
            if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            //nstr为空
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }else if(formatter_status == 1){
            std::cout << "pattern_error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern error>>", fmt, 0));
        }
    }

    if(!nstr.empty()){
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

}






















