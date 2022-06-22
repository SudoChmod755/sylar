#ifndef __SYLAR_BYTEARRAY_H__
#define __SYLAR_BYTEARRAY_H__
#include<memory>
#include<string>

namespace sylar{
    class ByteArray{
        public:
            typedef std::shared_ptr<ByteArray> ptr;
            struct Node{
                Node(size_t s);
                Node();
                ~Node();

                char* ptr;
                Node* next;
                size_t size;
            };
        private:

    };
}

#endif