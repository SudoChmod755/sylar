#include"log.h"
#include<map>
#include<functional>
namespace sylar{

    const char* LogLevel:: ToString (LogLevel:: Level level){
        switch(level){
#define XX(name) \
    case LogLevel:: name: \
        return #name; \
        break;
        
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);

#undef  XX
        default: 
            return "UNKNOW";
        }
    }
//-----------------


    LogEventWrap:: LogEventWrap(LogEvent:: ptr event):m_event(event) {}

     std::stringstream& LogEventWrap::  getSS(){
        return m_event->getss();
    }

    LogEventWrap:: ~LogEventWrap(){
            m_event->getLogger()->log(m_event->getLevel(),m_event);
        };




//-----------------

    Logger:: Logger(const std:: string& name):m_name(name),m_level(LogLevel:: DEBUG)
        {
        m_formater.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }
    void Logger:: log(LogLevel::Level level,LogEvent:: ptr event){
        if(level>=m_level){
            auto self=shared_from_this();
            for(auto& i: m_appender ){
                i->log(self,level,event);
            }
        }
    }
    void Logger:: debug(LogEvent:: ptr event){
        log(LogLevel:: DEBUG,event);
    }
    void Logger:: info(LogEvent:: ptr event){
        log(LogLevel:: INFO,event);
    }
    void Logger:: warn(LogEvent:: ptr event){
        log(LogLevel:: WARN,event);
    }
    void Logger:: fatal(LogEvent:: ptr event){
        log(LogLevel:: FATAL,event);
    }
    void Logger:: error(LogEvent:: ptr event){
        log(LogLevel:: ERROR,event);
    }

    void Logger:: addappender(LogAppender:: ptr appender){
        if(!appender->getFormatter()){
            appender->setFormatter(m_formater);
        }
       Logger:: m_appender.push_back(appender);
       
    }
    void Logger:: delappender(LogAppender:: ptr appender){
        for(auto it=m_appender.begin();
        it!=m_appender.end();++it){
            if(*it==appender)   m_appender.erase(it);
        }
    }


//------------------


    FileLogAppender:: FileLogAppender(const std:: string& fname):m_filename(fname) {
        m_filestream.open(m_filename);
    }

    void FileLogAppender:: log(Logger:: ptr pt, LogLevel:: Level level,LogEvent:: ptr event)  {
        if(level>=m_level){
            m_filestream<< m_logformatter->format(pt,event,level);
        }
    }
    
    bool FileLogAppender:: reopen(){
        if(m_filestream){
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }

    void StdoutAppender:: log(Logger:: ptr pt,LogLevel:: Level level,LogEvent:: ptr event)  {
        if(level>=m_level){
            std:: cout<<m_logformatter->format(pt,event,level);
        }
    }


//-------------------------
    class MessgeFormaItem : public LogFormatter::LogFormatItem{
        public:
        MessgeFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getcontent();
        }
    };

    class LogLevelFormaItem: public LogFormatter:: LogFormatItem{
        public:
        LogLevelFormaItem(const std::string& =""){}
        void format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
           out<< LogLevel:: ToString(level);
        }
    };

    class NameFormaItem : public LogFormatter::LogFormatItem{
        public:
        NameFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getfile();
        }
    };

    class TheardFormaItem : public LogFormatter::LogFormatItem{
        public:
        TheardFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getthreadid();
        }
    };

    class ElapseFormaItem : public LogFormatter::LogFormatItem{
        public:
        ElapseFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getelapse();
        }
    };

    

    class DateFormaItem : public LogFormatter::LogFormatItem{

        public:
        DateFormaItem(const std::string& format="%Y:%m:%d %H:%M:%S"):m_format(format){
            if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
        }
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            struct tm tm;
            time_t time=event->gettime();
            localtime_r(&time,&tm);
            char buf[64];
            strftime(buf,sizeof(buf),m_format.c_str(),&tm);
            out<< buf;
        }
        private:
        std:: string m_format;
        
    };

    class LognameFormaItem : public LogFormatter::LogFormatItem{
        public:
        LognameFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< pt->getName();
        }
    };

    class LinenbFormaItem : public LogFormatter::LogFormatItem{
        public:
        LinenbFormaItem(const std:: string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getline();
        }
    };

    class NewLinenbFormaItem : public LogFormatter::LogFormatItem{
        public:
        NewLinenbFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< std::endl;
        }
    };

    class StringFormaItem : public LogFormatter::LogFormatItem{
        public:
        StringFormaItem(const std:: string str):LogFormatItem(str),m_str(str){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< m_str;
        }
        private:
        std:: string m_str;
    };

    class TableFormaItem : public LogFormatter::LogFormatItem{
        public:
        TableFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< "\t";
        }
        private:
        std:: string m_str;
    };

    class FiberFormaItem : public LogFormatter::LogFormatItem{
        public:
        FiberFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getfiberid();
        }
    };

    class ThreadNameFormaItem : public LogFormatter::LogFormatItem{
        public:
        ThreadNameFormaItem(const std::string& =""){}
        void  format(Logger:: ptr pt,std:: ostream& out,LogEvent:: ptr event,LogLevel:: Level level) override{
            out<< event->getthreadName();
        }
    };



    LogFormatter:: LogFormatter(const std:: string& forma ): m_pattern(forma) {
        init();
    }

    std:: string LogFormatter:: format(Logger:: ptr pt,LogEvent:: ptr event,LogLevel:: Level level){
        std:: stringstream ss;
        for( auto& i:m_items){
            i->format(pt,ss,event,level);
        }
        return ss.str();
    }

    //   %xxxx  %xxx{xxx} %%
    //   [%d] [%p] (%f:%l) - %m %n
    void LogFormatter::  init(){
        //str fmt type
        std:: vector<std:: tuple<std:: string ,std:: string ,int>> vec;
        std:: string nstr;
        for(size_t i=0;i<m_pattern.size();i++){
            if(m_pattern[i]!='%'){
                nstr.append(1,m_pattern[i]);
                continue;
            }

            if(i+1<m_pattern.size()){
                if(m_pattern[i+1]=='%'){
                    nstr.append(1,'%');
                    continue;
                }
            }

            size_t n=i+1;
            int forma_stg=0;
            size_t forma_begin=0;

            std:: string str;
            std:: string fmt;

            while(n<m_pattern.size()){
                if(!isalpha(m_pattern[n]) &&
                 m_pattern[n]!='{' &&
                 m_pattern[n]!='}' && !forma_stg  ) {
                    str=m_pattern.substr(i+1,n-i-1);
                    break;
                }
                if(forma_stg==0){
                    if(m_pattern[n]=='{'){
                        str=m_pattern.substr(i+1,n-1-i);
                        forma_stg=1;
                        forma_begin=n;
                        n++;
                        continue;
                    }
                    
                }
                if(forma_stg==1){
                    if(m_pattern[n]=='}'){
                        fmt=m_pattern.substr(forma_begin+1,n-forma_begin-1);
                        forma_stg=0;
                        ++n;
                        break;        
                    }
                }

                n++;
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

            if(forma_stg==0){
                if(!nstr.empty()){
                    vec.push_back(std:: make_tuple(nstr,"",0));
                    nstr.clear();
                }
                vec.push_back(std:: make_tuple(str,fmt,1));
                i=n-1;
            }
            else if(forma_stg==1){
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                // m_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }
        
        if(!nstr.empty()){
            vec.push_back(std:: make_tuple(nstr,"",0));
        }
        //%m -- 消息体
        //%p -- level
        //%r -- 启动后的时间
        //%d --  时间
        //%t -- 线程id
        //%c -- 日志名称
        //%f -- 文件名
        //%l -- 行号
        //%n -- 回车换行

        static std:: map<std:: string ,std::function<LogFormatItem:: ptr(const std:: string& s)> >  s_format_items= {
        #define XX(st,C) \
          {  #st,[](const std:: string& fmt){return LogFormatItem:: ptr(new C(fmt)); } }


        XX(m,MessgeFormaItem),      //日志内容
        XX(p,LogLevelFormaItem),    //日志级别
        XX(r,ElapseFormaItem),      //已经启动的时间
        XX(d,DateFormaItem),        //时间戳
        XX(t,TheardFormaItem),      //线程id
        XX(c,LognameFormaItem),     //日志名
        XX(n,NewLinenbFormaItem),   //换行符
        XX(l,LinenbFormaItem),      //行号
        XX(f,NameFormaItem),        //文件名 
        XX(T,TableFormaItem),       //制表符
        XX(F,FiberFormaItem),        //协程id
        XX(N,ThreadNameFormaItem)    //线程名字
        #undef  XX
    };
        

        for(auto& i : vec){
            if(std:: get<2>(i)==0){
                m_items.push_back(LogFormatItem::ptr(new StringFormaItem( std:: get<0>(i) )));
            }
            else
            {
                auto it=s_format_items.find(std:: get<0>(i));
                if(it==s_format_items.end()){
                    m_items.push_back(LogFormatItem::ptr(new StringFormaItem( "<<error format % "+std:: get<0>(i)+" >> " )));
                }
                else{
                    m_items.push_back(it->second(std:: get<1>(i)));

                }
            }
            //std:: cout<<std:: get<0>(i)<<" -- "<<std:: get<1>(i)<< " -- "<< std:: get<2>(i)<< std:: endl; 

        }

    }
    

    //-----------------------------

    LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char*file,int32_t line,uint32_t thread,
            uint32_t elapse,u_int32_t fiber,uint64_t time,std:: string threadName):m_logger(logger),m_level(level),
            m_file(file),m_line(line),m_threadId(thread),m_elapse(elapse),m_fiberId(fiber),
            m_time(time),m_threadName(threadName)
            {}

    void LogEvent:: format(const char* fmt, ...){
        va_list al;
        va_start(al,fmt);
        format(fmt,al);
        va_end(al);

    }

    void LogEvent:: format(const char* fmt,va_list al){
        char* buf=nullptr;
        int len=vasprintf(&buf,fmt,al);
        if(len!=-1){
            m_ss<<std::string(buf,len);
            free(buf);            //UAF?
            //printf("%d",buf);
        }
    }

    //-----------------------------------

    LogerManger:: LogerManger(){
        m_root.reset(new Logger);
        m_root->addappender(LogAppender:: ptr(new StdoutAppender));
        m_loggers[m_root->getName()]=m_root;
        init();

    }

    void LogerManger::init(){

    }

    Logger:: ptr LogerManger:: getLogger(const std:: string& name){
        auto it=m_loggers.find(name);
        if(it!=m_loggers.end()){
            return it->second;
        }
        Logger:: ptr logger(new Logger(name));
        logger->setRoot(m_root);
        m_loggers[name]=logger;
        return logger;     
    }

    


    

}

