#ifndef __SYLAR_MACRO_H
#define __SYLAR_MACRO_H

#include<string.h>
#include<assert.h>
#include "util.h"

#if defined __GNUC__ || defined __llvm__
#   define  SYLAR_LICKLY(x)  __builtin_expect(!!(x),1)
#   define  SYLAR_UNLICKLY(x) __builtin_expect(!!(x),0)
#else
#   define  SYLAR_LICKLY(x) (x)
#   define  SYLAR_ULICKLY(x) (x)
#endif

#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLICKLY(!(x))){  \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"Assertion: " #x \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100,2,"    "); \
        assert(x); \
    }

#define SYLAR_ASSERT2(x,w) \
    if(SYLAR_UNLICKLY(!(x))){  \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"Assertion: " #x \
            << "\n"<< w \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100,2,"    "); \
        assert(x); \
    }





#endif