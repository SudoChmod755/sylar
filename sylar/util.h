#ifndef __SYLAR_UTIL_H_
#define __SYLAR_UTIL_H_

#include<sys/syscall.h>
#include<pthread.h>
#include<stdio.h>
#include<sys/types.h>
#include <unistd.h>
#include<stdint.h>

namespace sylar{
    pid_t GetThreadId();
    uint64_t GetFiberId();
}






#endif