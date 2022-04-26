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

#define SYLAR_LOG_LEVEL(logger,level) \
        if(logger->getlevel()<=level) \
            sylar:: LogEventWrap(sylar:: LogEvent:: ptr(new sylar::LogEvent(logger,level, \
            __FILE__,__LINE__,sylar::GetThreadId(),1,sylar::GetFiberId(),time(0),"szyshs"))).getSS()

#define SYLAR_LOG_DEBUG(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)

#define SYLAR_LOG_INFO(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)

#define SYLAR_LOG_WARN(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)

#define SYLAR_LOG_ERROR(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)

#define SYLAR_LOG_FATAL(logger)  SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)

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
    };

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
                return m_ss;
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

    };

    
    //日志输出地
    class LogAppender{

        public:
            typedef std:: shared_ptr<LogAppender> ptr;
            virtual ~LogAppender() {}
            virtual void log(std:: shared_ptr<Logger> pt,  LogLevel:: Level level,LogEvent:: ptr event)=0;
            void setFormatter(LogFormatter:: ptr val){
                m_logformatter=val;
            }
            LogFormatter:: ptr getFormatter(){
                return m_logformatter;
            }
        protected:
            LogLevel:: Level m_level;
            LogFormatter:: ptr m_logformatter;

    };


    //日志器
    class Logger: public std:: enable_shared_from_this<Logger>{
       public:
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
        private:            
            std:: string m_name;   //日志名称
            LogLevel:: Level m_level;    //日志级别
            std:: list<LogAppender:: ptr> m_appender; //Appender集合  
            LogFormatter:: ptr m_formater;
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
    };

    
}

#endif
