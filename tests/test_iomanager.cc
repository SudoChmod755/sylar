#include "sylar/sylar.h"
#include "sylar/iomanager.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include <arpa/inet.h>


sylar::Logger::ptr g_logger=SYLAR_LOG_ROOT();

void test_fiber(){
    SYLAR_LOG_INFO(g_logger)<<"test_fiber";
}

void test1(){
    sylar::IOManger iom;
    iom.schedule(&test_fiber);

    int sock=socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(80);
    inet_pton(AF_INET,"180.101.49.11",&addr.sin_addr.s_addr);

    iom.addEvent(sock,sylar::IOManger::WRITE,[](){
        SYLAR_LOG_INFO(g_logger)<<"connected";
    });

    connect(sock,(const sockaddr*)&addr,sizeof(addr));
}
int main(int argc,char** argv){
    test1();
    return 0;
}