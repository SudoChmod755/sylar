#include "sylar/sylar.h"

sylar::Logger::ptr g_logger=SYLAR_LOG_ROOT();

void test_fiber(){
    static int s_count=5;
    SYLAR_LOG_INFO(g_logger)<<"test in fiber s_count="<<s_count;
    sleep(1);
    if(--s_count>=0)
    {
        sylar::Scheduler::GetThis()->schedule(&test_fiber,sylar::GetThreadId());
        std::cout<<std::endl;
    }
    
}
int main(int argc,char** argv){
    sylar::Scheduler sc(3,false,"test");
    sc.start();
    sc.schedule(&test_fiber);
    SYLAR_LOG_INFO(g_logger)<<"willin stopp";
    sc.stop();
    SYLAR_LOG_INFO(g_logger)<<"over";
    return 0;
}