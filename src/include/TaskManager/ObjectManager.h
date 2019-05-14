/*
================================
### class ObjectManager : 
    管理有名共享内存区和互斥锁等
### Constructor:
    静态调用
### Interfaces:
================================
*/
#ifndef ObjectManager_H_
#define ObjectManager_H_
#include "headers.h"

class ObjectManager {
private:
    static std::unordered_map<std::string, std::string> mem_map;
    static std::mutex M;

public:
    static std::vector<std::pair<std::string, size_t>> get_mem_keysize();
    static bool exist_mem(const char* mem_name);
    static bool create_mem(const char* mem_name, size_t size);
    static std::string get_mem(const char* mem_name);
};

#endif
