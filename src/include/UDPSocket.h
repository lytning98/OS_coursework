/*
================================
### class UDPSocket : 
    使用 UDP 完成 UNIX 域上的 Socket 通信
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

/*  初始化为 server
    [arg1] : domain file 路径                               */
    UDPSocket(const char* server_filepath) 
        : is_server(true), server_file(server_filepath) {}

/*  初始化为 client
    [arg1] : server domain file 地址
    [arg2] : 自身 domain file 地址                          */
    UDPSocket(const char* server_filepath, const char* client_filepath)
        : is_server(false), server_file(server_filepath), client_file(client_filepath) {}

/*  发送数据
    [data] : 待发送 T 实例
    [return] : 成功(true)或否, 出错时设置 errno             */
    template<typename T>
    bool send(T data) {
        return this->send(&data, sizeof(data));
    }

/*  接收数据
    [data] : 用于保存接收数据的 T 类型实例
    [return] : 成功(true)或否, 出错时设置 errno             */
    template<typename T>
    bool recv(T& data) {
        return this->recv(&data, sizeof(data));
    }

/*  初始化, 建立 socket 完成 bind
    出错时设置 errno
    [return] : 成功(true)或否, 出错时设置 errno             */
    bool initialize();
};

#endif
