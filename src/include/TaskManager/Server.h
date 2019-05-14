/*
================================
### class Server : 
    保存和处理与一个 Server(ServerGuard) 有关的信息
### Constructor:
    Server(fd, IP) : 
        初始化, 启动监听进程
        [fd] : TCP file descriptor
        [IP] : IP 地址
### Public members:
    id : 自动分配的编号
    IP : IP 地址
    task_name : 正在执行的 Task
### Interfaces:
    bool launch(filepath) :
        在该服务器上启动任务
        出错时自动输出错误信息
        [filepath] : zip 文件路径
        [return] : 成功(true)与否, 下同
================================
*/
#ifndef Server_H_
#define Server_H_

#include "headers.h"
#include "TCPShared.h"

class Server {
private:
    int fd;
    std::thread thread;

    void watch();
    void create_named_mem(const packet& pack);
    void request_data(const packet& pack);
    void write_named_mem(const packet& pack);

public:
    static int id_top;
    int id;
    std::string IP, task_name;
    bool busy = false, online = true;

    Server() {};

    Server(int fd, const char* IP) : fd(fd), id(++id_top) {
        this->IP = std::string(IP);
        this->thread = std::thread([this]() -> void {this->watch();});
    }

    bool launch(const char* filepath);
};

#endif
