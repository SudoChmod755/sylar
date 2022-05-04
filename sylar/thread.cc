#include "thread.h"
#include "log.h"
#include "util.h"
namespace sylar{
    static thread_local Thread* t_thread=nullptr;
    static thread_local std::string t_thread_name="UNKNOW";

    static Logger::ptr g_logger=SYLAR_LOG_NAME("system");

    Semaphore::Semaphore(uint32_t count){
        if(sem_init( &m_semaphore, 0, count )){
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore(){
            sem_destroy(&m_semaphore);
    }
    void Semaphore::wait(){
        if(sem_wait(&m_semaphore)){
            throw std::logic_error("sem_wait error");
        }
    }

    void Semaphore::notify(){
        if(sem_post(&m_semaphore)){
            throw std::logic_error("sem_post error");
        }
    }


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

        m_semaphore.wait();
    }

    void* Thread::run(void *arg){
        Thread* temp=(Thread*) arg;
        t_thread=temp;
        t_thread_name=temp->m_name;
        temp->m_id=sylar::GetThreadId();
        pthread_setname_np(pthread_self(),temp->m_name.substr(0,15).c_str());
        std::function<void ()> cb;
        cb.swap(temp->m_cb);

        temp->m_semaphore.notify();
        cb();
        return 0;
    }
}