#ifndef __SYLAR_TIMER_H_
#define __SYLAR_TIMER_H_
#include<memory>
#include "thread.h"
#include<set>
#include<vector>
namespace sylar{
    class TimerManager;
    class Timer: public std::enable_shared_from_this<Timer> {
        friend class TimerManager;
        public:
            typedef std::shared_ptr<Timer> ptr;
            bool cancel();
            bool refresh();
            bool reset(uint64_t ms,bool from_now);
        private:
            Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* manager);
            Timer(uint64_t next);
        private:
            bool m_recurring=false; //是否循环定时器
            uint64_t m_ms=0;        //循环周期
            uint64_t m_next=0;      //精确定时时间
            std::function<void()> m_cb;
            TimerManager* m_manager=nullptr;
        private:
            struct Comparetor{
                bool operator() (const Timer::ptr& lsh,const Timer::ptr& rsh) const ;
            };
    };

    class TimerManager{
        friend class Timer;
        public:
            typedef RWMutex RWMutexType;
            TimerManager();
            virtual ~TimerManager();
            Timer::ptr addtimer(uint64_t ms,std::function<void()> cb,bool recurring=false);
            Timer::ptr addConditionTimer(uint64_t ms,std::weak_ptr<void> weak_cond, std::function<void()> cb,bool recurring=false);
            uint64_t getNextTimer();
            void listExpiredCb(std::vector<std::function<void()> >& cbs);
            bool hasTimer();
        protected:
            virtual void onTimerInsertedAtFront() = 0;
            void addTimer(Timer::ptr val,RWMutexType::WriteLock& lock);
        private:
            bool detectClockRollover(uint64_t now_ms);
        private:
            RWMutex m_mutex;
            std::set<Timer::ptr,Timer::Comparetor> m_timers;   //是类型而不是成员.
            bool m_tickled=false;
            uint64_t m_previouseTime=0;
    };


}

#endif