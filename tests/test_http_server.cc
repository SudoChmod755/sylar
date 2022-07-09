#include "sylar/http/http_server.h"
#include "sylar/log.h"
#include "sylar/tcp_server.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include "sylar/bytearray.h"
#include "sylar/sylar.h"
static sylar::Logger::ptr g_logger=SYLAR_LOG_ROOT();

void run(){
    sylar::http::HttpServer::ptr  server(new sylar::http::HttpServer);
    sylar::Address::ptr addr=sylar::Address::LookupAnyIPAdd("0.0.0.0:8020");
    while(!server->bind(addr)){
        sleep(2);
    }
    auto sd=server->getServletDispatch();
    sd->addServlet("/sylar/xx",[](sylar::http::HttpRequest::ptr request,
        sylar::http::HttpResponse::ptr response,
        sylar::http::HttpSession::ptr session){
            response->setBody(request->toString());
            return 0;
        });

     sd->addGlobServlet("/sylar/*",[](sylar::http::HttpRequest::ptr request,
        sylar::http::HttpResponse::ptr response,
        sylar::http::HttpSession::ptr session){
            response->setBody("Glob:\r\n"+request->toString());
            return 0;
        });
    server->start();
}
int main(){
    sylar::IOManger iom(2);
    iom.schedule(run);
    return 0;
}