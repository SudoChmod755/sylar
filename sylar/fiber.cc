#include "fiber.h"
#include <stdint.h>
#include <atomic>
#include "config.h"
#include "macro.h"
#include "log.h"
#include "scheduler.h"
namespace sylar{
static Logger::ptr g_logger =SYLAR_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id{0};
static std::atomic<uint64_t> s_fiber_count{0};

static thread_local Fiber* t_fiber=nullptr;
static thread_local Fiber::ptr t_threadFiber=nullptr;

static ConfigVar<uint32_t>:: ptr g_fiber_stack_size=
        Config::Lookup<uint32_t>("fiber.stack_size",1024*1024,"fiber stack size");


class MallocStackAllocator{
    public:
        static void* Alloc(::size_t size){
            return malloc(size);
        }

        static void Dealloc(void* vp,::size_t size){
            return free(vp);
        }
};


using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId(){
    if(t_fiber){
        return t_fiber->getId();
    }
    return 0;
}

Fiber::Fiber(){
    m_state=EXEC;
    SetThis(this);            //这个是当前进程的主线作为协程。

    if(getcontext(&m_ctx)){               //这个地方？？
        SYLAR_ASSERT2(false,"getcontext");
    }

    ++s_fiber_count;
    SYLAR_LOG_DEBUG(g_logger)<<"Fiber::Fiber";
};

Fiber::Fiber(std::function<void()> cb,size_t stacksize,bool use_caller):m_id(++s_fiber_id),
    m_cb(cb)
    {
        ++s_fiber_count;
        m_stacksize=stacksize? stacksize: g_fiber_stack_size->getVal();  //没有传参就用配置的
        m_stack =StackAllocator::Alloc(m_stacksize);
        if(getcontext(&m_ctx)){
           SYLAR_ASSERT2(false,"getcontext");
        }
        m_ctx.uc_link=nullptr;
        m_ctx.uc_stack.ss_sp=m_stack;
        m_ctx.uc_stack.ss_size=m_stacksize;
        if(use_caller){
            makecontext(&m_ctx,&Fiber::CallerMainFunc,0);
        }
        else{
            makecontext(&m_ctx,&Fiber::MainFunc,0);
        }
        SYLAR_LOG_DEBUG(g_logger)<<"Fiber::Fiber"<<" id: "<<m_id;
    }
Fiber::~Fiber(){
    --s_fiber_count;
    if(m_stack){
        SYLAR_ASSERT(m_state==TERM
        || m_state==INIT
        || m_state==EXCEP)

        StackAllocator::Dealloc(m_stack,m_stacksize);
    }
    else{
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state=EXEC);

        Fiber* cur=t_fiber;
        if(cur==this){
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger)<<"~Fiber::Fiber"<<" id: "<<m_id;
}
//重置协程函数，并重置状态
//INIT,TERM
void Fiber::reset(std::function<void()> cb){
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state==TERM || m_state==INIT
    ||m_state==EXCEP);
    m_cb=cb;
    if(getcontext(&m_ctx)){
        SYLAR_ASSERT2(false,"getcontext");
    }
    m_ctx.uc_link=nullptr;
    m_ctx.uc_stack.ss_sp=m_stack;
    m_ctx.uc_stack.ss_size=m_stacksize;
    makecontext(&m_ctx,&Fiber::MainFunc,0);
    m_state=INIT;
}

void Fiber::call(){
    SetThis(this);
    m_state=EXEC;
    if(swapcontext(&t_threadFiber->m_ctx,&m_ctx)) {
        SYLAR_ASSERT2(false,"swapcontext");
    }
}

void Fiber::back(){
    
    
        SetThis(t_threadFiber.get());
        if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)){
        SYLAR_ASSERT2(false,"swapcontext");
        }
    
}

//切换到当前协程执行
void Fiber::swapIn(){
    SetThis(this);
    SYLAR_ASSERT(m_state !=EXEC);
    m_state=EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx , &m_ctx)){
        SYLAR_ASSERT2(false,"swapcontext");
    }

}
//切换到后台执行
void Fiber::swapOUt(){
    
    
        SetThis(Scheduler::GetMainFiber());
        if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)){
        SYLAR_ASSERT2(false,"swapcontext");
        }
   
}
//设置当前协程
void Fiber::SetThis(Fiber* f){
    t_fiber=f;
}
//返回当前协程
Fiber:: ptr Fiber::GetThis(){
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);        //私有构造函数只能在内部调用(shared_from_this前必须要初始化对象)
    SYLAR_ASSERT(t_fiber==main_fiber.get());
    t_threadFiber=main_fiber;
    return t_fiber->shared_from_this();
}
//协程切换到后台，并且设置为Ready状态
void Fiber::YieldToReady(){
    Fiber:: ptr cur=GetThis();
    cur->m_state=READY;
    cur->swapOUt();
};
//协程切换到后台，并且设置为Hold状态
void Fiber::YieldToHold(){
    SYLAR_LOG_INFO(g_logger)<<"yield";
    Fiber:: ptr cur= GetThis();
    cur->m_state=HOLD;
    cur->swapOUt();
}
//总协程数
uint64_t Fiber::TotalFibers(){
    return s_fiber_count;
}

void Fiber::MainFunc(){
    Fiber::ptr cur =GetThis();
    SYLAR_ASSERT(cur);
    try{
        cur->m_cb();           //run_in_fiber
        cur->m_cb=nullptr;
        cur->m_state=TERM;
    }catch(std::exception& ex){
        cur->m_state=EXCEP;
        SYLAR_LOG_ERROR(g_logger)<<"Fiber Except: "<<ex.what()
        <<" fiber_id="<<cur->getId()
        <<std::endl<<sylar::BacktraceToString();
    }catch(...){
        cur->m_state=EXCEP;
        SYLAR_LOG_ERROR(g_logger)<<"Fiber Except: "
         <<" fiber_id="<<cur->getId()
        <<std::endl<<sylar::BacktraceToString();
    }
    auto raw_ptr=cur.get();
    cur.reset();
    raw_ptr->swapOUt();
    SYLAR_ASSERT2(false,"never reach fiber_id=" +std::to_string(raw_ptr->getId()));    //如果不加前两行 没有析构Fiber对象的原因是
                                           //协程的上下文切换时没有引用计数减一 (因为没有析构 cur对象)
                                           //如果去除swapout 则这个线程直接结束 没有机会对main函数中的 智能指针对象析构

}

void Fiber::CallerMainFunc(){
    Fiber::ptr cur =GetThis();
    SYLAR_ASSERT(cur);
    try{
        cur->m_cb();           //run_in_fiber
        cur->m_cb=nullptr;
        cur->m_state=TERM;
    }catch(std::exception& ex){
        cur->m_state=EXCEP;
        SYLAR_LOG_ERROR(g_logger)<<"Fiber Except: "<<ex.what()
        <<" fiber_id="<<cur->getId()
        <<std::endl<<sylar::BacktraceToString();
    }catch(...){
        cur->m_state=EXCEP;
        SYLAR_LOG_ERROR(g_logger)<<"Fiber Except: "
         <<" fiber_id="<<cur->getId()
        <<std::endl<<sylar::BacktraceToString();
    }
    auto raw_ptr=cur.get();
    cur.reset();
    raw_ptr->back();
    SYLAR_ASSERT2(false,"never reach fiber_id=" +std::to_string(raw_ptr->getId()));
}


}