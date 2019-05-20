/*
================================
### class ObjectManager : 
    管理有名共享内存区和互斥锁等
### Constructor:
    静态调用
### Interfaces:
    见下
================================
*/
#ifndef ObjectManager_H_
#define ObjectManager_H_
#include "headers.h"

class ObjectManager {
private:
    static std::unordered_map<std::string, std::string> mem_map;
    static std::unordered_map<std::string, std::mutex> mutex_map;
    static std::mutex M, M_mutex;

public:
    /*  获取所有内存区大小信息 
        [return]    key-value vector, value 为内存区大小    */
    static std::vector<std::pair<std::string, size_t>> get_mem_keysize();

    /*  获取指定内存区大小
        [mem_name]  内存区名
        [return]    大小                                    */
    static size_t get_mem_size(const char* mem_name);

    /*  检测指定内存区是否存在
        [mem_name]  内存区名
        [return]    存在(true)或否(false)                   */
    static bool exist_mem(const char* mem_name);

    /*  创建内存区
        [mem_name, szie]  内存区名和大小
        [return]          成功(true)或否(false)             */
    static bool create_mem(const char* mem_name, size_t size);

    /*  获取内存区内数据
        [mem_name]  内存区名
        [return]    内存区内数据, 出错时返回空字节串          */
    static std::string get_mem(const char* mem_name);

    /*  写入内存区
        [mem_name, data]    内存区名和待写入数据
        [return]    成功(true)或否(false)                   */
    static bool write_mem(const char* mem_name, const std::string& data);

    /*  保存内存区内数据至本地
        [mem_name]  内存区名
        [filepath]  保存路径
        [return]    成功(true)或否(false)                   */
    static bool save_mem_to(const char* mem_name, const char* filepath);
    
    /*  删除有名内存区
        [mem_name]  内存区名
        [return]    成功(true)或否(false)                   */
    static bool del_mem(const char* mem_name);

    /*  获取所有互斥锁的状态信息 
        [return]    key-value vector, value 为互斥锁状态     */
    static std::vector<std::pair<std::string, bool>> get_mutex_keystate();

    /*  检测指定名称的互斥锁是否存在
        [mut_name]  互斥锁名称
        [return]    存在(true)或否(false)                   */
    static bool exist_mutex(const char* mut_name);

    /*  创建互斥锁
        [mut_name]  互斥锁名称
        [return]    成功(true)或否(false)                   */
    static bool create_mutex(const char* mut_name);

    /*  LOCK 互斥锁
        [mut_name]  互斥锁名称
        [return]    成功(true)或否(false)                   */
    static bool lock_mutex(const char* mut_name);

    /*  UNLOCK 互斥锁
        [mut_name]  互斥锁名称
        [return]    成功(true)或否(false)                   */
    static bool unlock_mutex(const char* mut_name);
};

#endif
