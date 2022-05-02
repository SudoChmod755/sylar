#include "thread.h"
#include "log.h"
#include "util.h"
namespace sylar{
    static thread_local Thread* t_thread=nullptr;
    static thread_local std::string t_thread_name="UNKNOW";

    static Logger::ptr g_logger=SYLAR_LOG_NAME("system");

    Thread* Thread::GetThis(){
        return t_thread;
    }

    const std:: string& Thread::GetName(){
        return t_thread_name;
    }

    Thread:: ~Thread(){
        if(m_thread){
            pthread_detach(m_thread);
        }
    }

    void Thread:: SetName(const std:: string& name){
        t_thread_name=name;
        if(t_thread){
            t_thread->m_name=name;
        }
    }

    void Thread:: join(){
        if(m_thread){
            int rt=pthread_join(m_thread,nullptr);    
            if(rt){
                SYLAR_LOG_ERROR(g_logger)<<" pthread_join thread fail,rt="<<rt<<" name="<<m_name;
                throw std::logic_error("pthread join error");
            }
            m_thread=0;
        }
    }


    Thread::Thread(std::function<void ()> cb,const std:: string& name ):m_cb(cb),m_name(name)
    {
        if(name.empty()) {
            m_name="UNKNOW";
        }
        int rt=pthread_create(&m_thread,nullptr,&Thread::run,this);
        if(rt){
            SYLAR_LOG_ERROR(g_logger)<<" pthread creat thread fail,rt="<<rt <<" name= "<<name;
            throw std::logic_error("pthread create error");
        }
    }

    void* Thread::run(void *arg){
        Thread* temp=(Thread*) arg;
        t_thread=temp;
        t_thread_name=temp->m_name;
        temp->m_id=sylar::GetThreadId();
        pthread_setname_np(pthread_self(),temp->m_name.substr(0,15).c_str());
        std::function<void ()> cb;
        cb.swap(temp->m_cb);
        cb();
        return 0;
    }
}