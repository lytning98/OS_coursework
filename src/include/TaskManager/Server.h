/*
================================
### class Server : 
    保存和处理与一个 Server(ServerGuard) 有关的信息
### Constructor:
    Server(fd, IP) : 
        初始化, 启动监听进程
        [fd] : TCP file descriptor
        [IP] : IP 地址
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
    //  自动分配的编号
    int id;
    //  IP 地址
    std::string IP;
    //  正在运行的 Task 名称
    std::string task_name;
    //  Server 是否有正在运行的 Task
    bool busy = false;
    //  Server 是否仍在线
    online = true;

    Server() {};

    Server(int fd, const char* IP) : fd(fd), id(++id_top) {
        this->IP = std::string(IP);
        this->thread = std::thread([this]() -> void {this->watch();});
    }

/*  在该服务器上启动任务, 出错时自动输出错误信息
    [filepath] : zip 文件路径
    [return] : 成功(true)与否 */
    bool launch(const char* filepath);
};

#endif
