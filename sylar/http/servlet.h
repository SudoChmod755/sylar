#ifndef __SYLAR_SERVLET_H__
#define __SYLAR_SERVLET_H__
#include<memory>
#include<functional>
#include<string>
#include"http.h"
#include"http_session.h"
#include<map>
#include<unordered_map>
#include<vector>
#include"sylar/thread.h"
namespace sylar{
    namespace http{
        class Servlet{
            public:
                typedef std::shared_ptr<Servlet> ptr;
                Servlet(const std::string& name):m_name(name) {}
                virtual ~Servlet() {}
                virtual int32_t handle(sylar::http::HttpRequest::ptr request,
                sylar::http::HttpResponse::ptr response,
                sylar::http::HttpSession::ptr session)=0;

                const std::string& getName() const {return m_name;}
            protected:
                std::string m_name;
        };

        class FunctionServlet: public Servlet{
            public:
                typedef std::shared_ptr<FunctionServlet> ptr;
                typedef std::function<int32_t (sylar::http::HttpRequest::ptr request,sylar::http::HttpResponse::ptr response,
                sylar::http::HttpSession::ptr session)> callback;
                FunctionServlet(callback cb);
                virtual int32_t handle(sylar::http::HttpRequest::ptr request,
                sylar::http::HttpResponse::ptr response,
                sylar::http::HttpSession::ptr session) override;
            private:
                callback m_cb;
        };

        class ServletDispatch: public Servlet{
            public:
                typedef std::shared_ptr<ServletDispatch> ptr;
                typedef RWMutex RWMutexType;
                virtual int32_t handle(sylar::http::HttpRequest::ptr request,
                sylar::http::HttpResponse::ptr response,
                sylar::http::HttpSession::ptr session)  override;
                ServletDispatch();

                void addServlet(const std::string& uri,Servlet::ptr slt);
                void addServlet(const std::string& uri,FunctionServlet::callback cb );
                void addGlobServlet(const std::string& uri,Servlet::ptr slt);
                void addGlobServlet(const std::string& uri,FunctionServlet::callback cb );

                void delServlet(const std::string& uri);
                void delGlobServlet(const std::string& uri);

                Servlet::ptr getDefault() const {return m_default;}
                void setDefault(Servlet::ptr v) {m_default=v;}

                Servlet::ptr getServlet(const std::string& uri) ;
                Servlet::ptr getGlobServlet(const std::string& uri);

                Servlet::ptr getMatchedServlet(const std::string& uri);
            private:
                std::unordered_map<std::string,Servlet::ptr> m_datas;

                //正则模糊匹配。
                std::vector<std::pair<std::string,Servlet::ptr> > m_globs;

                Servlet::ptr m_default;

                RWMutexType m_mutex;
        };


        class NotFoundServlet: public Servlet{
            public:
                typedef std::shared_ptr<NotFoundServlet> ptr;
                NotFoundServlet();
                virtual int32_t handle(sylar::http::HttpRequest::ptr request,
                sylar::http::HttpResponse::ptr response,
                sylar::http::HttpSession::ptr session)  override;
                
        };
    }
}

#endif