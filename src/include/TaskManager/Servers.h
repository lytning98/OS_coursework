#ifndef Servers_H_
#define Servers_H_

#include <string>
#include <vector>

namespace Servers {
    // 用于提供服务器列表信息的结构体
    struct server_info {
        int id, fd;
        bool online;
        bool busy;
        const std::string& IP;
        const std::string& task_name;
        server_info (int id, int fd, bool online, bool busy, const std::string& IP, const std::string& task_name)
            : id(id), fd(fd), online(online), busy(busy), IP(IP), task_name(task_name) {}
    };

/*  获取互斥锁, 用于在模块外对服务器信息进行互斥操作
    [return]      std::unique_lock<mutex> 的右值引用            */
    std::unique_lock<std::mutex> && get_lock();

/*  添加服务器
    [fd]        file descriptor
    [IP]        Server IP                                      */
    void add_server(int fd, const char* IP);

/*  寻找合适的服务器启动 Task
    [filepath]      zip file path (task package)
    [silence]       失败时是否输出错误信息
    [return]        成功(0)或errcode(>0)
    [error]         无可用服务器(1), 无法打开文件(2), TCP发送失败(3), zip文件发送失败(4)    */
    int launch_task(const char* filepath);

/*  获取服务器列表信息
    [return]    vector of struct{id, online, busy, IP, task_name}                       */
    std::vector<server_info> get_server_list();

/*  断开与所有服务器的连接                                      */
    void disconnect_all();
}

#endif
