#ifndef __SYLAR_HTTP_HTTP_SERVER_H__
#define __SYLAR_HTTP_HTTP_SERVER_H__
#include "sylar/tcp_server.h"
#include "servlet.h"


namespace sylar{
    namespace http{
        class HttpServer:public TcpServer{
            public:
                typedef std::shared_ptr<HttpServer> ptr;
                HttpServer(bool keepalive=false,
                            sylar::IOManger* worker=sylar::IOManger::GetThis()
                            ,sylar::IOManger* accept_worker=sylar::IOManger::GetThis());

                ServletDispatch::ptr getServletDispatch() const {return m_dispatch;}
                void setServletDispatch(ServletDispatch::ptr sd) {m_dispatch=sd;}
            protected:
                virtual void handleClient(Socket::ptr client);
            private:
                bool m_isKeepalive;
                ServletDispatch::ptr m_dispatch;
        };
    }
}

#endif