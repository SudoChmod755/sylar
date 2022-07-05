#ifndef __SYLAR_HTTP_PARSER_H__
#define __SYLAR_HTTP_PARSER_H__
#include<memory>
#include<string>
#include<map>
#include<boost/lexical_cast.hpp>
#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http.h"
namespace sylar{
namespace http{

    class HttpRequestParser{
    public:
        typedef std::shared_ptr<HttpRequestParser> ptr;
        HttpRequestParser();

        size_t execute(char* data,size_t len);
        int isFinished() ;
        int hasError() ;

        HttpRequest::ptr getData() const {return m_data;}
        void setError(int v) {m_error=v;}

        uint64_t getContentLength();
    private:
        http_parser m_parser;
        HttpRequest::ptr m_data;
        //1000 invalid method
        int m_error;
};

class HttpResponseParser{
    public:
        typedef std::shared_ptr<HttpResponseParser> ptr;
        HttpResponseParser();
        size_t execute(char* data,size_t len);
        int isFinished() ;
        int hasError() ;
        HttpResponse::ptr getData() const {return m_data;}
        void setError(int v) {m_error=v;}
        uint64_t getContentLength();
    private:
        httpclient_parser m_parser;
        HttpResponse::ptr m_data;
        //1001: invalid version
        int m_error;

};


}
}

#endif