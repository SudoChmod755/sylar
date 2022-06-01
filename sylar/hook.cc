#include"hook.h"
#include"fiber.h"
#include"iomanager.h"
#include<dlfcn.h>
#include"fd_manager.h"
#include"sylar.h"
namespace sylar{
    sylar::Logger::ptr g_logger=SYLAR_LOG_NAME("system");
    static thread_local bool t_hook_enable=false;

    #define HOOK_FUN(XX) \
        XX(sleep) \
        XX(usleep) \
        XX(nanosleep) \
        XX(socket) \
        XX(connect) \
        XX(accept) \
        XX(read) \
        XX(readv) \
        XX(recv) \
        XX(recvfrom) \
        XX(recvmsg) \
        XX(write) \
        XX(writev) \
        XX(send) \
        XX(sendto) \
        XX(sendmsg) \
        XX(close) \
        XX(fcntl) \
        XX(ioctl) \
        XX(getsockopt) \
        XX(setsockopt)


    void hook_init(){
       static bool is_inited=false;
        if(is_inited){
            return;
        }


    #define XX(name) name ## _f =(name ## _fun)dlsym(RTLD_NEXT,#name);   //返回符号对应的地址(指针)。 //定义
        HOOK_FUN(XX)
    #undef XX
    }

    struct _HookIniter{
        _HookIniter(){
            hook_init();
        }
    };

    static _HookIniter s_hook_initer;   

    bool is_hook_enable(){
        return t_hook_enable;
    }
    void set_hook_enable(bool flag){
        t_hook_enable=flag;
    }
}

    struct timer_info{
        int cancelled=0;
    };

    template<typename OriginFun,typename ... Args>
    static ssize_t do_io(int fd,OriginFun fun,const char* hook_fun_name,
    uint32_t event,int timeout_so,ssize_t buflen,Args&&... args){            //右值   //ssize_t 是 signed size_t
        if(!sylar::t_hook_enable){
            return fun(fd,std::forward<Args>(args)...);
        }

        sylar::FdCtx::ptr ctx=sylar::FdMgr::GetInstance()->get(fd);
        if(!ctx){
            return fun(fd,std::forward<Args>(args)...);
        }

        if(ctx->isClose()){
            errno=EBADF;
            return -1;
        }

        if(!ctx->isSocket() || ctx->getUserNonblock()){  //用户非阻塞
            return fun(fd,std::forward<Args>(args)...);
        }

        uint64_t to =ctx->getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);
retry:
        ssize_t n =fun(fd,std::forward<Args>(args)...);
        while(n==-1 && errno=EINTR){
            n=fun(fd,std::forward<Args>(args)...);
        }
        if(n==-1 && errno=EAGAIN){     //EAGAIN非阻塞读写失败（比如没有数据可读）
            sylar::IOManger* iom=sylar::IOManger::GetThis();
            sylar::Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo);
            if(to!=(uint64_t)-1){
                timer=iom->addConditionTimer(to,winfo,[winfo,fd,iom,event](){
                    auto t=winfo.lock();
                    if(!t || t->cancelled){
                        return;
                    }
                    t->cancelled=ETIMEDOUT;
                    iom->cancelEvent(fd,(sylar::IOManger::Event)(event));
                })
            }
            int c=0;
            uint64_t now=0;

            int rt=iom->addEvent(fd,(sylar::IOManger::Event)(event));
            if(rt){
                
                SYLAR_LOG_ERROR(g_logger)<<hook_fun_name<<" addEvent("<<fd<<", "<<event<<")";
                if(timer){
                    timer->cancel();
                }
                return -1;
            }
            else{
                sylar::Fiber::YieldToHold();    //epoll_wait事件会schedule当前fiber,那时候回来。 也有可能是超时了回来？？
                if(timer){
                    timer->cancel();
                }
                if(tinfo->cancelled){
                    errno=tinfo->cancelled;
                    return -1;
                }

                goto retry;
            }
        

        }
        return n;
    }


extern "C"{
    #define XX(name) name ## _fun name ## _f=nullptr;
        HOOK_FUN(XX)
    #undef XX           //声明

    

    unsigned int sleep(unsigned int seconds){
        if(!sylar::t_hook_enable){
            return sleep_f(seconds);
        }

        sylar::Fiber::ptr fiber=sylar::Fiber::GetThis();        //主线程的协程。
        sylar::IOManger* iom=sylar::IOManger::GetThis();
        //iom->addtimer(seconds*1000,std::bind(&sylar::IOManger::schedule,iom,fiber));
        iom->addtimer(seconds*1000,[iom,fiber](){
            iom->schedule(fiber);
        });
        sylar::Fiber::YieldToHold(); 
        return 0;
    }

    int usleep(useconds_t usec){
        if(!sylar::t_hook_enable){
            return usleep_f(usec);
        }

        sylar::Fiber::ptr fiber=sylar::Fiber::GetThis();
        sylar::IOManger* iom=sylar::IOManger::GetThis();
        //iom->addtimer(usec / 1000,std::bind(&sylar::IOManger::schedule,iom,fiber));
        iom->addtimer(usec/1000,[iom,fiber](){
            iom->schedule(fiber);
        });
        sylar::Fiber::YieldToHold(); 
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem){
        if(!sylar::t_hook_enable){
            return nanosleep_f(req,rem);
        }

        int timeout_ms =req->tv_sec * 1000+req->tv_nsec/1000/1000;
        sylar::Fiber::ptr fiber=sylar::Fiber::GetThis();
        sylar::IOManger* iom=sylar::IOManger::GetThis();
        iom->addtimer(timeout_ms,[iom,fiber](){
            iom->schedule(fiber);
        });
        sylar::Fiber::YieldToHold(); 
        return 0;
    }

}


// typedef unsigned int (*sleep_fun)(unsigned int seconds);
// extern sleep_fun sleep_f;
// typedef int (*usleep_fun)(useconds_t usec);
// extern usleep_fun usleep_f;