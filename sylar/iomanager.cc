#include "iomanager.h"
#include "sylar.h"
#include "fcntl.h"
#include <error.h>
namespace sylar{
    static sylar::Logger:: ptr g_logger=SYLAR_LOG_NAME("system");

    IOManger::FdContext::EventContext& IOManger::FdContext::getContext(IOManger::Event event){
        switch(event){
            case IOManger::READ:
                return m_read;
            case IOManger::WRITE:
                return m_write;
            default:
                SYLAR_ASSERT2(false,"getContext");
        }
    }


    void IOManger::FdContext::resetContext(IOManger::FdContext::EventContext& ctx){
        ctx.scheduler=nullptr;
        ctx.fiber.reset();
        ctx.cb=nullptr;
    }

    void IOManger::FdContext::triggerEvent(IOManger::Event event){
        SYLAR_ASSERT(m_events & event);
        m_events=(Event)(m_events & ~event);
        EventContext& ctx=getContext(event);
        if(ctx.cb){
            ctx.scheduler->schedule(&ctx.cb);
        }
        else{
            ctx.scheduler->schedule(&ctx.fiber);
        }

        ctx.scheduler=nullptr;
        return;
    }



    IOManger:: IOManger(size_t threads,bool use_caller,const std::string& name)
    :Scheduler(threads,use_caller,name)
    {
        m_epfd=epoll_create(5000);
        SYLAR_ASSERT(m_epfd>0);
        int rt=pipe(m_tickleFds);
        SYLAR_ASSERT(!rt);
        epoll_event event;
        memset(&event,0,sizeof(epoll_event));
        event.events=EPOLLIN | EPOLLET;  //边沿触发。
        event.data.fd=m_tickleFds[0];      //监听tickleFds[0]上的EPOLLIN事件。

        rt=fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);    //非阻塞io,没得读写返回-1.
        SYLAR_ASSERT(!rt);

        rt=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
        SYLAR_ASSERT(!rt);
        contextResize(32);
        start();
    } //设置成员变量epfd，监听成员变量tickle_fd[0](管道)可读事件。



    IOManger::~IOManger(){
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);
        for(size_t i=0;i<m_fdContexts.size();++i){
            if(m_fdContexts[i]){
                delete m_fdContexts[i];
            }
        }
    }

    void IOManger::contextResize(size_t size){
        m_fdContexts.resize(size);

        for(size_t i=0;i<m_fdContexts.size();i++){
            if(!m_fdContexts[i]){
                m_fdContexts[i]=new FdContext;
                m_fdContexts[i]->fd=i;
            }
        }

    }    


    //1 success , 0 retry, -1 error
    int IOManger::addEvent(int fd,Event event,std::function<void()> cb){
        FdContext* fd_ctx=nullptr;

        RWMutexType::ReadLock  lock(m_mutex);
        if( (int)m_fdContexts.size()> fd){
            fd_ctx=m_fdContexts[fd];
            lock.unlock();
        }
        else{
            lock.unlock();
            RWMutex::WriteLock lock2(m_mutex);
            contextResize(fd*1.5);
            fd_ctx=m_fdContexts[fd];
        }

        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if(fd_ctx->m_events & event){
            SYLAR_LOG_ERROR(g_logger)<<"addEvent assert fd="<<fd
            <<" event="<<event
            <<" fd_ctx.event"<<fd_ctx->m_events;

            SYLAR_ASSERT(!(fd_ctx->m_events & event));
            
        }               //不能是相同的

        int op=fd_ctx->m_events? EPOLL_CTL_MOD:EPOLL_CTL_ADD;     //原先有没有事件。
        epoll_event epevent;
        epevent.events=EPOLLET | fd_ctx->m_events | event;
        epevent.data.ptr =fd_ctx;     //可以知道在哪个FdContext触发。(将epoll和fd_ctx关联起来)
    
        int rt =epoll_ctl(m_epfd,op,fd,&epevent);
        if(rt){
            SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<rt<< " ("<<errno<<") ("<<strerror(errno)<<")";
            return -1;
        }

        ++m_pendingEventCount;
        fd_ctx->m_events=(Event)(fd_ctx->m_events | event);
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        SYLAR_ASSERT(!event_ctx.scheduler
        && !event_ctx.fiber 
        && !event_ctx.cb);
 
        event_ctx.scheduler = Scheduler::GetThis();
        if(cb){
            event_ctx.cb.swap(cb);
        }
        else{
            event_ctx.fiber=Fiber::GetThis();
            SYLAR_ASSERT(event_ctx.fiber->getState()==Fiber::EXEC);
        }

        return 0;


    }


    bool IOManger::delEvent(int fd,Event event){
        RWMutexType::ReadLock lock(m_mutex);
        if( (int) m_fdContexts.size()<=fd){
            return false;
        }
        FdContext* fd_ctx=m_fdContexts[fd];
        lock.unlock();
        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if(!(fd_ctx->m_events & event)){
            return false;
        }

        Event new_events= (Event)(fd_ctx->m_events& ~event);
        int op=new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events=EPOLLET | new_events;
        epevent.data.ptr=fd_ctx;
        
        int rt=epoll_ctl(m_epfd,op,fd,&epevent);
        if(rt){
            SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<rt<< " ("<<errno<<") ("<<strerror(errno)<<")";
            return false;
        }
        --m_pendingEventCount;
        fd_ctx->m_events=new_events;
        FdContext::EventContext& event_ctx=fd_ctx->getContext(event);
        fd_ctx->resetContext(event_ctx);
        return true;
    }

    bool IOManger::cancelEvent(int fd,Event event){
        RWMutexType::ReadLock lock(m_mutex);
        if( (int) m_fdContexts.size()<=fd){
            return false;
        }
        FdContext* fd_ctx=m_fdContexts[fd];
        lock.unlock();
        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if(!(fd_ctx->m_events & event)){
            return false;
        }

        Event new_events= (Event)(fd_ctx->m_events& ~event);
        int op=new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events=EPOLLET | new_events;
        epevent.data.ptr=fd_ctx;
        
        int rt=epoll_ctl(m_epfd,op,fd,&epevent);
        if(rt){
            SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<rt<< " ("<<errno<<") ("<<strerror(errno)<<")";
            return false;
        }
        fd_ctx->triggerEvent(event);
        --m_pendingEventCount;
        return true;
    }


    bool IOManger::cancelAll(int fd){
        RWMutexType::ReadLock lock(m_mutex);
        if((int)m_fdContexts.size()<=fd){
            return false;
        }
        FdContext* fd_ctx=m_fdContexts[fd];
        lock.unlock();
        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if(!fd_ctx->m_events){
            return false;
        }

        
        int op=EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events=0;
        epevent.data.ptr=fd_ctx;
        
        int rt=epoll_ctl(m_epfd,op,fd,&epevent);
        if(rt){
            SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
            <<op<<","<<fd<<","<<epevent.events<<"):"
            <<rt<< " ("<<errno<<") ("<<strerror(errno)<<")";
            return false;
        }

        if(fd_ctx->m_events & READ){
            fd_ctx->triggerEvent(READ);
            --m_pendingEventCount;
        }

        if(fd_ctx->m_events & WRITE){
            fd_ctx->triggerEvent(WRITE);
            --m_pendingEventCount;
        }

        SYLAR_ASSERT(fd_ctx->m_events==0);
        return true;
    }

    IOManger* IOManger::GetThis(){
        return dynamic_cast<IOManger*>(Scheduler::GetThis());
    }

    void IOManger::tickle() {      
        if(hasIdleThreads()){    //没有idlethread
            return;
        }
        int rt=write(m_tickleFds[1],"T",1);
        SYLAR_ASSERT(rt==1);

    }

    bool IOManger::stopping(uint64_t& timeout){
        timeout=getNextTimer();
        return timeout==~0ull 
        && m_pendingEventCount==0
        && Scheduler::stopping();
    }

    bool IOManger::stopping() {
        uint64_t timeout=0;
        return stopping(timeout);
    }


    void IOManger::idle() {
        epoll_event* events=new epoll_event[64]();
        std::shared_ptr<epoll_event> shared_events(events,[](epoll_event* ptr){
            delete[] ptr;
        });

        while(true){
            uint64_t next_timeout=0;
            if(stopping(next_timeout)){
                
                    SYLAR_LOG_INFO(g_logger)<<"name="<<getName()
                    <<" idle stopping exit";
                    break;
                }
            

            

            int rt=0;
            do{
                static const int MAX_TIMEOUT=5000;
                if(next_timeout !=~0ull){
                    next_timeout= (int) next_timeout>MAX_TIMEOUT? MAX_TIMEOUT:next_timeout;
                }
                else{
                    next_timeout=MAX_TIMEOUT;
                }
                rt=epoll_wait(m_epfd,events,64,(int)next_timeout);    //前面已经epoll_ctl了。
                
                if(rt<0 && errno==EINTR){
                    //系统中断。。。
                }
                else{
                    break;
                }
            }while(true);                                 //epoll_wait.



            std::vector<std::function<void()> >cbs;
            listExpiredCb(cbs);
            if(!cbs.empty()){
                schedule(cbs.begin(),cbs.end());
                cbs.clear();
            }


             for(int i=0;i<rt;++i){
                 epoll_event& event=events[i];
                 if(event.data.fd==m_tickleFds[0]){
                     uint8_t dummy;
                     while(read(m_tickleFds[0],&dummy,1)==1);
                     continue;
                 }

                 FdContext* fd_ctx=(FdContext* )event.data.ptr;      //fd不是 tickleFDs[0]
                 FdContext::MutexType::Lock lock(fd_ctx->mutex);
                 if(event.events & (EPOLLERR | EPOLLHUP) ){
                     event.events |= EPOLLIN | EPOLLOUT;
                 }

                 int real_events=NONE;
                 if(event.events & EPOLLIN){
                     real_events |= READ;
                 }

                 if(event.events & EPOLLOUT){
                     real_events |= WRITE;
                 }

                 if((fd_ctx->m_events & real_events)==NONE){
                     continue;
                 }

                 int left_events=(fd_ctx->m_events & ~real_events);  //real_events没有但是m_events中有的事件。
                 int op=left_events? EPOLL_CTL_MOD: EPOLL_CTL_DEL;
                 event.events=EPOLLET | left_events;

                 int rt2=epoll_ctl(m_epfd,op,fd_ctx->fd,&event);
                 if(rt2){
                     SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "
                    <<op<<","<<fd_ctx->fd<<","<<event.events<<"):"
                    <<rt<< " ("<<errno<<") ("<<strerror(errno)<<")"; 
                    continue;   
                 }

                 if(real_events & READ){
                     fd_ctx->triggerEvent(READ);     //会调用schedule（也许会tickle)。
                     --m_pendingEventCount;
                 }

                 if(real_events & WRITE){
                     fd_ctx->triggerEvent(WRITE);
                     --m_pendingEventCount;
                 }

             }


             Fiber::ptr cur=Fiber::GetThis();
             auto raw_ptr=cur.get();
             cur.reset();

             raw_ptr->swapOUt();   //或者用yieldtohold(意味着没结束)。
        }

       

    }


    void IOManger::onTimerInsertedAtFront() {
        tickle();
    } 


}