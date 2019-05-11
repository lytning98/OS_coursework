/*
================================
### class TCPSocket : 
    使用 TCP 完成 Internet 上的 Socket 通信
### Constructor:
    TCPSocket(port) : 
        初始化为 server
        [port] : 监听端口
    TCPSocket(server_IP, server_port) : 
        初始化为 client
        [arg1] : server 地址
        [arg2] : server 端口
    [!] 构造函数仅完成数据设置, 初始化操作在 initialize 中完成, 方便获取初始化结果

### Interfaces:
    bool initialize() :
        初始化, Server 完成 bind, listen; Client 完成 connect.
        出错时设置 errno, 下同.
        [return] : 成功(true)与否, 下同
    bool send(T data) :
        发送数据, 参考 UDPSocket 类
        [data] : 待发送 T 实例
    bool recv(T& data) :
        阻塞接收 T 类型的数据
        [data] : 用于保存接收数据的 T 类型实例
    int accept():
        阻塞等待新连接
        [return] : 连接的 fd, -1 为失败
================================
*/
#ifndef TCPSocket_H_
#define TCPSocket_H_
#include "headers.h"

class TCPSocket {
private:
    const char *server_IP;
    struct sockaddr_in server_addr, client_addr;
    int fd, port;
    bool is_server;

    bool init();
    bool init_server();
    bool init_client();
public:
    TCPSocket() {}
    TCPSocket(int port) : 
        is_server(true), port(port) {}
    TCPSocket(const char* server_IP, int port) :
        is_server(false), port(port), server_IP(server_IP) {}
    
    template<typename T>
    bool send(T data, int fd = -1) {
        if(!this->is_server)    fd = this->fd;
        return ::send(fd, &data, sizeof(data), 0) != -1;
    }

    template<typename T>
    bool recv(T& data, int fd = -1) {
        if(!this->is_server)    fd = this->fd;
        return ::recv(fd, &data, sizeof(data), 0);
    }

    int accept();
    bool initialize();
    const char* get_client_IP();
};

#endif
