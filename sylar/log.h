#ifndef __SYLAR_LOG_H_
#define __SYLAR_LOG_H_

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<sstream>
#include<fstream>
#include<iostream>
#include<vector>
#include<tuple>
#include<ctime>
#include<cstdarg>
#include<map>
#include "singleton.h"
#include "thread.h"

#define SYLAR_LOG_LEVEL(logger,level) \
        if(logger->getlevel()<=level) \
            sylar:: LogEventWrap(sylar:: LogEvent:: ptr(new sylar::LogEvent(logger,level, \
            __FILE__,__LINE__,sylar::GetThreadId(),1,sylar::GetFiberId(),time(0),sylar::Thread::GetName()))).getSS()

#define SYLAR_LOG_DEBUG(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)

#define SYLAR_LOG_INFO(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)

#define SYLAR_LOG_WARN(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)

#define SYLAR_LOG_ERROR(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)

#define SYLAR_LOG_FATAL(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)


//   用格式化字符串输入log message
#define SYLAR_FORMAT_LEVEL(logger,level,fmt,...) \
        if(logger->getlevel()<=level)  \
            sylar:: LogEventWrap(sylar::LogEvent:: ptr(new sylar:: LogEvent(logger,level, \
            __FILE__,__LINE__,sylar::GetThreadId(),1,sylar::GetFiberId(),time(0),sylar::Thread::GetName() ))).getEvent()->format(fmt,__VA_ARGS__) 

#define SYLAR_FORMAT_DEBUG(logger,fmt, ...)  SYLAR_FORMAT_LEVEL(logger,sylar::LogLevel::DEBUG,fmt, __VA_ARGS__)

#define SYLAR_FORMAT_INFO(logger,fmt, ...)  SYLAR_FORMAT_LEVEL(logger,sylar::LogLevel::INFO,fmt, __VA_ARGS__)

#define SYLAR_FORMAT_WARN(logger,fmt, ...)  SYLAR_FORMAT_LEVEL(logger,sylar::LogLevel::WARN,fmt, __VA_ARGS__)

#define SYLAR_FORMAT_ERROR(logger,fmt, ...)  SYLAR_FORMAT_LEVEL(logger,sylar::LogLevel::ERROR,fmt, __VA_ARGS__)

#define SYLAR_FORMAT_FATAL(logger,fmt, ...)  SYLAR_FORMAT_LEVEL(logger,sylar::LogLevel::FATAL,fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT()  sylar::LogerMgr::GetInstance()->getRoot()

#define SYLAR_LOG_NAME(name)  sylar::LogerMgr::GetInstance()->getLogger(name)

namespace sylar{

    //日志级别
    class LogLevel{
        public:
            enum Level{
                UNKNOW=0,
                DEBUG=1,
                INFO=2,
                WARN=3,
                ERROR=4,
                FATAL=5
            };
        
        static const char* ToString (LogLevel:: Level level);
        static const Level FromString(const std::string& str);
    };
    class LogerManger;
    class Logger;
    //日志事件
    class LogEvent{
        public:
            typedef std::shared_ptr<LogEvent> ptr;

            LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char*file,int32_t line,uint32_t thread,
            uint32_t elapse,u_int32_t fiber,uint64_t time,std:: string threadName);

            const char* getfile() const {
                return m_file;
            }

            int32_t getline() const {
                return m_line;
            }

            uint32_t getthreadid() const {
                return m_threadId;
            }

            uint32_t getelapse() const {
                return m_elapse;
            }

            uint32_t getfiberid() const {
                return m_fiberId;
            }

            std:: string getcontent() const {
                return m_ss.str();
            }

            std:: uint64_t gettime() const {
                return m_time;
            }

            std:: stringstream& getss()  {
                return m_ss;                  //这里是用来获取输入的
            }

            std:: string getthreadName() const {
                return m_threadName;
            }

            std:: shared_ptr<Logger> getLogger() const{
                return m_logger;
            }

            LogLevel:: Level getLevel() const {
                return m_level;
            }

            void format(const char* fmt, ...);

            void format(const char* fmt,va_list al);
            

        private:
            std:: shared_ptr<Logger> m_logger;  //日志器
            LogLevel:: Level m_level;           //日志级别
            const char* m_file=nullptr;      //文件名
            int32_t m_line=0;                //行号
            uint32_t m_threadId=0;           //线程id
            uint32_t m_elapse= 0;            //程序启动开始到现在的毫秒数
            uint32_t m_fiberId=0;            //协程id   
            uint64_t m_time;                 //时间戳
            std::stringstream m_ss;          //内容
            std::string m_threadName;        //线程名称
    };

    class LogEventWrap{
        public:
        LogEventWrap(LogEvent:: ptr event);

        ~LogEventWrap();

        LogEvent:: ptr getEvent() const {return m_event;}

        std::stringstream& getSS();
        

        private:
        LogEvent:: ptr m_event;   
    };



    //日志格式器
    class LogFormatter{
        public:
            typedef std:: shared_ptr<LogFormatter> ptr;
            std:: string format(std:: shared_ptr<Logger> pt, LogEvent:: ptr event,LogLevel:: Level level);
            LogFormatter(const std:: string& forma );
            void init();
            bool isError() const {
                return m_erro;
            }
        public:
            class LogFormatItem{
            public:
                typedef std:: shared_ptr<LogFormatItem> ptr;
                LogFormatItem(const std:: string fmt=""){};
                virtual ~LogFormatItem() {}
                virtual void format(std:: shared_ptr<Logger> pt ,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level)=0;
            };


            

        private:
             std:: list<LogFormatItem:: ptr> m_items;
             std:: string m_pattern;
             bool m_erro=false;

    };

    
    //日志输出地
    class LogAppender{
        friend class Logger;
        public:
            typedef Spinlock MutexType;
            typedef std:: shared_ptr<LogAppender> ptr;
            virtual ~LogAppender() {}
            virtual void log(std:: shared_ptr<Logger> pt,  LogLevel:: Level level,LogEvent:: ptr event)=0;
            void setFormatter(LogFormatter:: ptr val){
                MutexType::Lock lock(m_mutex);
                m_logformatter=val;
            }
            LogFormatter:: ptr getFormatter(){
                MutexType::Lock lock(m_mutex);
                return m_logformatter;
            }

            LogLevel:: Level getLevel(){
                return m_level;
            }

            void setLevel(LogLevel:: Level level){
                m_level=level;
            }


        protected:
            LogLevel:: Level m_level;
            LogFormatter:: ptr m_logformatter;
            MutexType m_mutex;

    };


    //日志器
    class Logger: public std:: enable_shared_from_this<Logger>{
       friend class LogerManger;
       public:
            typedef Spinlock MutexType;
            typedef std:: shared_ptr<Logger> ptr;
            Logger(const std:: string& name="root");
            void log(LogLevel::Level level,LogEvent:: ptr event);

            void debug(LogEvent:: ptr event);
            void info(LogEvent:: ptr event);
            void warn(LogEvent:: ptr event);
            void fatal(LogEvent:: ptr event);
            void error(LogEvent:: ptr event);

            void addappender(LogAppender:: ptr appender);
            void delappender(LogAppender:: ptr appender);
            void clearappender();
            void setFormatter(LogFormatter::ptr pt);
            void setFormatter(const std::string& s);
            LogFormatter::ptr getFormatter();

            LogLevel:: Level getlevel() const{return m_level;}
            void setlevel(LogLevel:: Level val) {
                m_level=val;
            }

            std:: string getName(){
                return m_name;
            }
            LogLevel:: Level getlevel(){
                return m_level;
            }

            void setRoot(Logger:: ptr root){
                m_root=root;
            }

        private:            
            std:: string m_name;   //日志名称
            LogLevel:: Level m_level;    //日志级别
            MutexType m_mutex;
            std:: list<LogAppender:: ptr> m_appender; //Appender集合  
            LogFormatter:: ptr m_formater;
            Logger:: ptr m_root;
    };

    class StdoutAppender : public LogAppender{
        public:
        typedef std:: shared_ptr<StdoutAppender> ptr;
        void log(std:: shared_ptr<Logger> pt ,LogLevel:: Level level,LogEvent:: ptr event) override;

    };

    class FileLogAppender : public LogAppender{

        public:
        typedef std:: shared_ptr<FileLogAppender> ptr;
        void log(std:: shared_ptr<Logger> pt,LogLevel:: Level level,LogEvent:: ptr event) override;
        FileLogAppender(const std:: string& fname);
        bool reopen();

        private:
        std:: string m_filename;
        std:: ofstream m_filestream;
        uint64_t m_lastTime=0;
    };

    class LogerManger{
        public:
            typedef Spinlock MutexType;
            LogerManger();
            Logger:: ptr  getLogger(const std:: string& name);
            void init();
            Logger:: ptr  getRoot() const  {return m_root;}
        private:
            MutexType m_mutex;
            Logger:: ptr m_root;
            std:: map<std::string,Logger:: ptr> m_loggers;
            
    };

    typedef sylar::SingleTonPtr<LogerManger> LogerMgr;
}

#endif
