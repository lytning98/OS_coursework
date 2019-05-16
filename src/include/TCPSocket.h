/*
================================
### class TCPSocket : 
    使用 TCP 完成 Internet 上的 Socket 通信
================================
*/
#ifndef TCPSocket_H_
#define TCPSocket_H_
#include "TCPShared.h"
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
    bool continuous_recv(int fd, std::function<void(const filepacket&)> callback);

    // 为客户端实例自动设置 fd, 为服务器端实例检查是否设置 fd
    inline void check_and_set_fd(int& fd) {
        if(!this->is_server)    fd = this->fd;
        assert(fd != -1);
    }
public:
    TCPSocket() {}

/*  初始化为 server
    [port] : 监听端口                                       */
    TCPSocket(int port) : 
        is_server(true), port(port) {}

/*  初始化为 client
    [arg1] : server 地址
    [arg2] : server 端口                                    */
    TCPSocket(const char* server_IP, int port) :
        is_server(false), port(port), server_IP(server_IP) {}
    
/*  发送数据
    [data] : 待发送 T 实例
    [fd]   : 服务器端需指定连接的文件描述符, 客户端无需指定
    [return] : 成功(true)或否, 出错时设置 errno             */
    template<typename T>
    bool send(T data, int fd = -1) {
        this->check_and_set_fd(fd);
        return ::send(fd, &data, sizeof(data), 0) != -1;
    }

/*  阻塞接收 T 类型的数据
    [data] : 用于保存接收数据的 T 类型实例
    [fd]   : 服务器端需指定连接的文件描述符, 客户端无需指定
    [return] : 成功(true)或否, 出错时设置 errno             */
    template<typename T>
    bool recv(T& data, int fd = -1) {
        this->check_and_set_fd(fd);
        return ::recv(fd, &data, sizeof(data), 0);
    }

/*  阻塞等待新连接
    [return] : 连接的 fd, -1 为失败                         */
    int accept();

/*  初始化, Server 完成 bind, listen; Client 完成 connect.
    [return] : 成功(true)或否, 出错时设置 errno             */
    bool initialize();

/*  获取最近建立连接的客户端 IP 地址
    [return] : IP 地址                                     */
    const char* get_client_IP();
    
/*  接收一段较大的数据
    [fd]   : 服务器端需指定连接的文件描述符, 客户端无需指定     
    [return] : 接收到的数据, 失败时返回空字节串              */
    std::string recv_large_data(int fd = -1);

/*  发送一段较大的数据
    [data] : 待发送数据
    [fd]   : 服务器端需指定连接的文件描述符, 客户端无需指定     
    [return] : 成功(true)或否, 出错时设置 errno             */
    bool send_large_data(const std::string& data, int fd = -1);

/*  接收文件
    [fd]   : 服务器端需指定连接的文件描述符, 客户端无需指定     
    [path] : 文件保存路径
    [return] : 成功(0), IO Error(1), TCP Error(2)          */
    int recv_file(const std::string& path, int conn_fd = -1);

/*  发送文件
    [fd]     : 服务器端需指定连接的文件描述符, 客户端无需指定
    [path]   : 待发送文件路径
    [return] : 成功(0), IO Error(1), TCP Error(2)          */
    int send_file(const std::string& path, int fd = -1);
};

#endif
