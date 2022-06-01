#include "sylar/hook.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/sylar.h"
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
void test_sleep(){
    sylar::IOManger iom(1);
    iom.schedule([](){
        sleep(2);
        SYLAR_LOG_INFO(g_logger)<<"sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        SYLAR_LOG_INFO(g_logger)<<"sleep 3";
    });

    SYLAR_LOG_INFO(g_logger)<<"test_sleep";


}
int main(int arg,char** argv){
    test_sleep();
    return 0;
}