#include "sylar/log.h"
#include "sylar/util.h"
int main(int argc,char** argv){
    sylar:: Logger:: ptr logger (new sylar:: Logger);
    /*
    堆区的指针（智能指针）
    这里初始化了  Logger及其成员（m_formatter....)
    formatter  init
      */
    logger->addappender(sylar:: LogAppender:: ptr(new sylar:: StdoutAppender));

    /*
    初始化了 logger中的appender成员
    同时也初始化了 appender中的成员 m_logformatter(就是logger的m_formatter 拷贝)
    */

    //sylar:: LogEvent:: ptr event(new sylar:: LogEvent(__FILE__,__LINE__,sylar::GetThreadId(),1,sylar::GetFiberId(),time(0),"szyshs"));
    //logger->log(sylar:: LogLevel:: DEBUG,event);

    /*
    ->appender->formatter的format函数
    最终将返回的str  输出到appender的输出空间
    */
   
    sylar:: FileLogAppender:: ptr fileappend(new sylar:: FileLogAppender("./log.txt")); 
    sylar:: LogFormatter:: ptr fmt(new sylar:: LogFormatter("%d%T%m%n"));
    fileappend->setFormatter(fmt);
    fileappend->setLevel(sylar:: LogLevel:: ERROR);
    logger->addappender(fileappend);

    
    std:: cout<<"hello sylar"<<std:: endl;
    SYLAR_LOG_INFO(logger)<<"hello sylar";
    SYLAR_FORMAT_DEBUG(logger,"test format %s debug","szyshs");
    SYLAR_LOG_ERROR(logger)<<"ERROR";
    std:: cout.flush();  
    auto l=sylar::LogerMgr::GetInstance()->getLogger("xx");
    l->addappender(sylar:: LogAppender:: ptr(new sylar:: StdoutAppender));
    SYLAR_LOG_DEBUG(l)<<"szyshs";
    //  logger和appender都有m_level   给定的level必须大于他们的才能输出

    return 0;

    
}