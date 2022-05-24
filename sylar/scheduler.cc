#include "scheduler.h"
#include "log.h"
#include "macro.h"
namespace sylar{
    static sylar::Logger::ptr g_logger=SYLAR_LOG_NAME("system");

    static thread_local Scheduler* t_scheduler=nullptr;

    static thread_local Fiber* t_fiber=nullptr;       //和fiber.cc中得t_fiber不是同一个，看相关代码在哪个文件里用哪个t_fiber

    Scheduler::Scheduler(size_t threads,bool use_caller,const std::string& name):m_name(name)
    {
        SYLAR_ASSERT(threads>0);
        if(use_caller){
            sylar::Fiber::GetThis();     //fiber.cc中的t_fiber
            --threads;
            SYLAR_ASSERT(GetThis()==nullptr);
            t_scheduler=this;
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));   //析构原来的FIBER对象（如果有）
            //  设置schedule主协程（不是private的那个main_fiber）
            sylar::Thread::SetName(m_name);
            t_fiber=m_rootFiber.get();
            m_rootThread=sylar::GetThreadId();
            m_threadIds.push_back(m_rootThread);

        }      //如果是use_caller,设置当前线程的主流是一个协程，设置run的m_rootFiber为另一个（保存在两个thread_local static 变量里）
        else{
            m_rootThread=-1;

        }
        m_threadCount=threads;
    }           //
    Scheduler:: ~Scheduler(){
        SYLAR_ASSERT(m_stopping);
        if(GetThis()== this){
            t_scheduler=nullptr;
        }
    }
    Scheduler* Scheduler::GetThis(){
        return t_scheduler;
    }
    Fiber* Scheduler:: GetMainFiber(){
        return t_fiber;
     }

     void Scheduler::start(){
         MutexType::Lock lock(m_mutex);
         if(!m_stopping){
             return;     //必须是停止得
         }
         m_stopping=false;
         SYLAR_ASSERT(m_threads.empty());
         m_threads.resize(m_threadCount);
         for(size_t i=0;i<m_threadCount;++i){
             m_threads[i].reset(new Thread( (std::bind(&Scheduler::run,this)) ,m_name+"_"+std::to_string(i)));  //这bind让其他线程可以访问到主线程栈上的sc(this参数)
             m_threadIds.push_back(m_threads[i]->getId());
         }
         lock.unlock();
        //  if(m_rootFiber){
        //      //m_rootFiber->swapIn();
        //      m_rootFiber->call(); //main_fiber保存在fiber.cc中，rootfiber（run）是m_rootfiber
        //      SYLAR_LOG_INFO(g_logger)<<"call out "<<m_rootFiber->getState();
        //  }
     }

     void Scheduler::stop(){
         m_autoStop=true;
         if(m_rootFiber && m_threadCount==0 && (m_rootFiber->getState()==Fiber::TERM || m_rootFiber->getState()
         ==Fiber::INIT) ){
             SYLAR_LOG_INFO(g_logger)<<this<<" stopped";
             m_stopping=true;
             if(stopping()){       //判断m_fiber是否为空 等。（stopping在子类中被重写）
                 return ;
             }
         }

         //bool exit_on_this_fiber=false;
         if(m_rootThread!=-1){
             SYLAR_ASSERT(GetThis()==this);  //只有use_call模式
             //在子类中这里的this指的是父类。（主要看函数在哪个类中实现的）。
             
         }
         else{
             SYLAR_ASSERT(GetThis()!=this);  
         }

         m_stopping=true;
         for(size_t i=0;i<m_threadCount;++i){
             tickle();   //子类中又会重写。
         }

         if(m_rootFiber ){
             tickle();
         }

         if(m_rootFiber){
            // while(!stopping()){
            //     if(m_rootFiber->getState()==Fiber::TERM || m_rootFiber->getState()==Fiber::EXCEP){
            //        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
            //        SYLAR_LOG_INFO(g_logger)<<" root fiber is term, reset";
            //        t_fiber=m_rootFiber.get();
            //     }
            //     m_rootFiber->call();
            // }
            if(!stopping()){       // 除了主协程以外的是否全部停止了(除了idle)
                m_rootFiber->call();
            }
         }

         std::vector<Thread::ptr> thrs;
         {
             MutexType::Lock lock(m_mutex);
             thrs.swap(m_threads);
         }

         for(auto& i: thrs){
             i->join();         //其他线程没结束时阻塞在这里。
         }

         if(stopping()){
             return;
         }
       
     }
     void Scheduler::setThis(){
         t_scheduler=this;
     }
     void Scheduler::run(){
        SYLAR_LOG_INFO(g_logger)<<"szyshs run";
        Fiber::GetThis();   
        setThis();
        if(sylar::GetThreadId()!=m_rootThread)
        {
            t_fiber=Fiber::GetThis().get();   //子线程swapout的切换。
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
        Fiber::ptr cb_fiber;
        FiberAndThread ft;
        while(true){
            ft.reset();
            bool tickle_me=false;
            bool is_active=false;
            {
                MutexType::Lock lock(m_mutex);
                auto it=m_fibers.begin();
                while(it!=m_fibers.end()){
                    if(it->thread!=-1 && it->thread!=sylar::GetThreadId()){
                        ++it;
                        tickle_me=true;
                        continue;
                    }
                    SYLAR_ASSERT(it->fiber || it->cb);
                    if(it->fiber && it->fiber->getState()==Fiber::EXEC){
                        ++it;
                        continue;
                    }
                    ft=*it;
                    m_fibers.erase(it);       //没有break也没++就double free
                    ++m_activeThreadCount;
                    is_active=true;
                    break;
                }
            }
            if(tickle_me){
                tickle();
            }
            if(ft.fiber && ft.fiber->getState()!=Fiber::TERM && ft.fiber->getState()!=Fiber::EXCEP){
                // ++m_activeThreadCount;
                ft.fiber->swapIn();
                --m_activeThreadCount;
                if(ft.fiber->getState()==Fiber::READY){
                    schedule(ft.fiber);
                }
                else if(ft.fiber->getState()!=Fiber::TERM && ft.fiber->getState()!=Fiber::EXCEP){
                    ft.fiber->m_state=Fiber::HOLD;
                }
                ft.reset();
            }

            else if(ft.cb){
                if(cb_fiber){
                    cb_fiber->reset(ft.cb);
                }
                else{
                    cb_fiber.reset(new Fiber(ft.cb));     //id=3
                }
                ft.reset();
                // ++m_activeThreadCount;
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if(cb_fiber->getState()== Fiber::READY){
                    schedule(cb_fiber);
                    cb_fiber.reset();      //已经拷贝了
                }
                else if(cb_fiber->getState()==Fiber::EXCEP || cb_fiber->getState()==Fiber::TERM){
                    cb_fiber->reset(nullptr);
                }else {//if(cb_fiber->getState()!=Fiber::TERM){
                    cb_fiber->m_state=Fiber::HOLD;
                    cb_fiber.reset();
                }
            }
            else{   
                if(is_active){
                    --m_activeThreadCount;
                    continue;
                }      
                if(idle_fiber->getState()==Fiber::TERM){
                    SYLAR_LOG_INFO(g_logger)<<"idle fiber term";
                    //continue;
                    break;
                }
                m_idleThreadCount++;
                idle_fiber->swapIn();        
                --m_idleThreadCount;
                if(idle_fiber->getState()!=Fiber::TERM  && idle_fiber->getState()!=Fiber::EXCEP){
                    idle_fiber->m_state=Fiber::HOLD;
                }
                
            }
        }
     }


    void Scheduler:: tickle(){
        SYLAR_LOG_INFO(g_logger)<<"tickle";
    }
    bool Scheduler:: stopping(){
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount==0;
    }
    void Scheduler:: idle(){
        SYLAR_LOG_INFO(g_logger)<<"idle";
        while(!stopping()){
            sylar::Fiber::YieldToHold();   //没有stop的时候会在run和这里循环
        }
    }


}