/*
================================
### class UDPSocket : 
    使用 UDP 完成 UNIX 域上的 Socket 通信
### Constructor:
    UDPSocket(server_filepath) : 
        初始化为 server
        [arg1] : domain file 路径
    UDPSocket(server_filepath, client_filepath) : 
        初始化为 client
        [arg1] : server 地址
        [arg2] : 自身地址

### Interfaces:
    bool initialize() :
        初始化, 出错时设置 errno, 下同
        [return] : 成功(true)与否, 下同
    bool send(T data) :
        发送数据
        [data] : 待发送 T 实例
    bool recv(T& data) :
        阻塞接收 T 类型的数据
        [data] : 用于保存接收数据的 T 类型实例
================================
*/

#ifndef UDPSocket_H_
#define UDPSocket_H_
#include "headers.h"

class UDPSocket {
private:
    const char  *server_file, *client_file;
    struct sockaddr_un server_addr, client_addr;
    int fd;
    bool is_server;

    bool init();
    bool init_server();
    bool init_client();
    bool send(void* buf, size_t len);
    bool recv(void* buf, size_t len);
public:
    UDPSocket() {}

    UDPSocket(const char* server_filepath) 
        : is_server(true), server_file(server_filepath) {}

    UDPSocket(const char* server_filepath, const char* client_filepath)
        : is_server(false), server_file(server_filepath), client_file(client_filepath) {}
    
    ~UDPSocket() {
        this->is_server ? unlink(server_file) : unlink(client_file);
    }

    template<typename T>
    bool send(T data){
        return this->send(&data, sizeof(data));
    }

    template<typename T>
    bool recv(T& data){
        return this-recv(&data, sizeof(data));
    }

    bool initialize();
};

#endif