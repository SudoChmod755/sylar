#include "timer.h"
#include "util.h"
namespace sylar{
    bool Timer::Comparetor:: operator() (const Timer::ptr& lsh,const Timer::ptr& rsh) const{
        if(!lsh && !rsh) return false;
        if(!lsh) return true;
        if(!rsh) return false;
        if(lsh->m_next< rsh->m_next) return true;
        if(lsh->m_next> rsh->m_next) return false;

        return lsh.get() < rsh.get();
    }

    Timer::Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* manager)
    :m_recurring(recurring),m_ms(ms),m_cb(cb),m_manager(manager)
    {
        m_next=GetCurrentMs()+m_ms;
    }

    Timer::Timer(uint64_t next):m_next(next){

    }

    bool Timer::cancel(){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(m_cb){
            m_cb=nullptr;
            auto it=m_manager->m_timers.find(shared_from_this());   //智能指针。
            m_manager->m_timers.erase(it);
            return true;
        }
        return false;
    }

    bool Timer::refresh(){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(!m_cb) return false;
        auto it=m_manager->m_timers.find(shared_from_this());
        if(it==m_manager->m_timers.end()) return false;
        m_manager->m_timers.erase(it);    //必须先删除再放回去，不能直接改m_next(因为set根据它来排序的)
        m_next=GetCurrentMs()+m_ms;
        m_manager->m_timers.insert(shared_from_this());
        return true;
    }
    bool Timer::reset(uint64_t ms,bool from_now){
        if(ms==m_ms && !from_now){
            return true;
        }
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(!m_cb) return false;
        auto it=m_manager->m_timers.find(shared_from_this());
        if(it==m_manager->m_timers.end()) return false;
        m_manager->m_timers.erase(it);
        uint64_t start=0;    
        if(from_now){
            start=GetCurrentMs();
        }
        else{
            start=m_next-m_ms;
        }
        m_ms=ms;
        m_next=start+m_ms;
        m_manager->addTimer(shared_from_this(),lock);
        return true;
    }


    TimerManager::TimerManager(){
        m_previouseTime=GetCurrentMs();
    }
    TimerManager::~TimerManager(){

    }
    Timer::ptr TimerManager::addtimer(uint64_t ms,std::function<void()> cb,bool recurring){
        Timer::ptr timer(new Timer(ms,cb,recurring,this));
        RWMutexType::WriteLock lock(m_mutex); 
        addTimer(timer,lock);
        return timer;
    }

    static void OnTimer(std::weak_ptr<void> weak_cond,std::function<void()> cb){
        std::shared_ptr<void> tmp=weak_cond.lock();      //判断智能指针管理的对象有没有释放。如果没有释放则得到shared_ptr否则为nullptr
        if(tmp){
            cb();
        }
    }

    Timer::ptr TimerManager::addConditionTimer(uint64_t ms,std::weak_ptr<void> weak_cond, std::function<void()> cb,bool recurring){
        return addtimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);
    }

    uint64_t TimerManager:: getNextTimer(){
        RWMutexType::ReadLock lock(m_mutex);
        m_tickled=false;
        if(m_timers.empty()){
            return ~0ull;
        }

        const Timer::ptr& next=*m_timers.begin();
        uint64_t now_ms=GetCurrentMs();
        if(now_ms>=next->m_next){
            return 0;
        }
        else{
            return next->m_next-now_ms;
        }


    }

     void TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs){    //epoll_wait之后就都过期了。
         uint64_t now_ms=GetCurrentMs();
         std::vector<Timer::ptr> expired;

         {
            RWMutexType::ReadLock lock(m_mutex);
            if(m_timers.empty()){
                return;
            }
         }

         RWMutexType::WriteLock lock(m_mutex);

         bool rollover=detectClockRollover(now_ms);
         if(!rollover && ((*m_timers.begin())->m_next> now_ms ) ){
             return;
         }

         Timer::ptr now_timer(new Timer(now_ms));
         auto it=rollover? m_timers.end() : m_timers.lower_bound(now_timer);
         while(it !=m_timers.end() && (*it)->m_next==now_ms){
             ++it;
         }
         expired.insert(expired.begin(),m_timers.begin(),it);
         m_timers.erase(m_timers.begin(),it);
         cbs.reserve(expired.size());
         for(auto& timer:expired){
             cbs.push_back(timer->m_cb);
             if(timer->m_recurring){
                 timer->m_next=now_ms+timer->m_ms;
                 m_timers.insert(timer);    //循环计时。
             }else{
                 timer->m_cb=nullptr;    //防止智能指针
             }
         }
     }

     void TimerManager::addTimer(Timer::ptr val,RWMutexType::WriteLock& lock){
        auto it=m_timers.insert(val).first;
        bool at_front=(it==m_timers.begin()) && !m_tickled;
        if(at_front){
            m_tickled=true;   //在设置epoll时间是会被置位false（刷新）
        }
        lock.unlock();

        if(at_front){
            onTimerInsertedAtFront();
        }
     }

     bool TimerManager::detectClockRollover(uint64_t now_ms){
         bool rollover=false;
         if(now_ms<m_previouseTime && now_ms<(m_previouseTime-60*60*1000)) {
             rollover=true;
         }
         m_previouseTime=now_ms;
         return rollover;
     }

     bool TimerManager::hasTimer(){
         RWMutexType::ReadLock lock(m_mutex);
         return !m_timers.empty();
     }
}