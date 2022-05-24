#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include "scheduler.h"
#include <sys/epoll.h>
#include "timer.h"
namespace sylar {
    
class IOManger : public Scheduler, public TimerManager{
        public:
            typedef std::shared_ptr<IOManger> ptr;
            typedef RWMutex RWMutexType;
            enum Event {
                NONE =0x0,
                READ =0x1,
                WRITE =0X4
            };
        private:
            struct FdContext
            {
                typedef Mutex MutexType;
                struct EventContext{
                    Scheduler* scheduler=nullptr;  //待执行的scheduler
                    Fiber::ptr fiber;       //事件协程
                    std::function<void()> cb;  //事件的回调函数
                };

                EventContext& getContext(Event event);
                void resetContext(EventContext& ctx);   //成员置零。
                void triggerEvent(Event event);

                int fd=0;             //事件关联的句柄
                EventContext m_read;  //读事件
                EventContext m_write; //写事件
                Event m_events= NONE; //已注册的事件
                MutexType mutex;
            };

        public:
            IOManger(size_t threads=1,bool use_caller=true,const std::string& name="");
            ~IOManger();    
            //1 success , 0 retry, -1 error
            int addEvent(int fd,Event event,std::function<void()> cb=nullptr);
            bool delEvent(int fd,Event event);
            bool cancelEvent(int fd,Event event);
            bool cancelAll(int fd);

            static IOManger* GetThis();
        
        protected:
            void tickle() override;
            bool stopping() override;
            bool stopping(uint64_t& timeout);
            void idle() override;
            void onTimerInsertedAtFront() override;
            void contextResize(size_t size);
        private:
            int m_epfd=0;           //成员是红黑树句柄。
            int m_tickleFds[2];

            std::atomic<size_t> m_pendingEventCount={0};
            RWMutexType m_mutex;
            std::vector<FdContext*> m_fdContexts;
    };
} 


#endif
