@startuml
Scheduler "1"*--"m" Scheduler::FiberAndThread
class Scheduler{
    -MutexType m_mutex
    -vector<Thread::ptr> m_threads
    -list<FiberAndThread> m_fibers
    -Fiber::ptr m_rootFiber
    -string m_name
    #vector<int> m_threadIds
    #size_t m_threadCount
    #atomic<size_t> m_activeThreadCount
    #atomic<size_t> m_idleThreadCount
    #bool m_stopping
    #bool m_autoStop
    #int m_rootThread

    +void start();
    +void stop();
    +void schedule()
    #void tickle();
    #void run();
    #bool stopping();
    #void idle();
    -bool scheduleNoLock<FiberOrCb>()
    
}

class Scheduler::FiberAndThread{
        +Fiber::ptr fiber;
        +function<void （）> cb;
        +int thread;
        +void reset()
}



@enduml
