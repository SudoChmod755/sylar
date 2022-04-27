#ifndef __SYLAR_SIGLET_H_
#define __SYLAR_SIGLET_H_
#include<memory>
namespace sylar{
    namespace {
      template<class T,class X,int N>
      T& GetInstanceX(){
          static T v;
          return v;
      }

      template<class T,class X,int N>
      std:: shared_ptr<T> GetInstancePtr(){
          static std:: shared_ptr<T> pt(new T);
          return pt;
      }
    }

    
    template<class T,class X=void,int N=0>
    class SingleTon{
        public:
            static T* GetInstance(){
                static T v;
                return &v;
            }

    };

    template<class T,class X=void,int N=0>
    class SingleTonPtr{
        public:
            static std:: shared_ptr<T> GetInstance(){
                static std:: shared_ptr<T> v(new T);
                return v;
            }
    };

}

#endif
