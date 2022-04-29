#ifndef __SYLAR_UTIL_H_
#define __SYLAR_UTIL_H_

#include<sys/syscall.h>
#include<pthread.h>
#include<stdio.h>
#include<sys/types.h>
#include <unistd.h>
#include<stdint.h>
#include<cxxabi.h>
namespace sylar{
    pid_t GetThreadId();
    uint64_t GetFiberId();

    template <class T>
    const char* TypeToname(){
        static const char* s_name=abi::__cxa_demangle(typeid(T).name(),nullptr,nullptr,nullptr);
        return s_name;
    }


}






#endif