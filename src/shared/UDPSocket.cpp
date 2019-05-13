/*
    UDPSocket 类的实现
*/
#include "UDPSocket.h"

bool UDPSocket::init() {
    this->server_addr.sun_family = AF_UNIX;
    strcpy(this->server_addr.sun_path, this->server_file);
    return (this->fd = socket(AF_UNIX, SOCK_DGRAM, 0)) != -1;
}

bool UDPSocket::init_server() {
    return this->init() && 
        (bind(this->fd, (struct sockaddr*)&this->server_addr, sizeof(this->server_addr)) != -1);
}

bool UDPSocket::init_client() {
    this->client_addr.sun_family = AF_UNIX;
    strcpy(this->client_addr.sun_path, this->client_file);
    if(!this->init()) {
        printf("Init server socket file failed.\n");
        return false;
    } else if(bind(this->fd, (struct sockaddr*)&this->client_addr, sizeof(this->client_addr)) == -1) {
        printf("Init socket (bind) failed.\n");
        return false;
    }
    return true;
}

bool UDPSocket::initialize() {
    return this->is_server ? this->init_server() : this->init_client();
}

bool UDPSocket::send(void* buf, size_t len) {
    auto dest = this->is_server ? 
        (struct sockaddr*)&this->client_addr : (struct sockaddr*)&this->server_addr;
    return sendto(this->fd, buf, len, 0, dest, sizeof(*dest)) != -1;
}

bool UDPSocket::recv(void* buf, size_t len) {
    if(is_server){
        static socklen_t tmp = sizeof(this->client_addr);
        return recvfrom(this->fd, buf, len, 0, (struct sockaddr*)&this->client_addr, &tmp) != -1;
    }else{
        return recvfrom(this->fd, buf, len, 0, NULL, NULL) != -1;
    }
}
