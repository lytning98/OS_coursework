#include "headers.h"
#include "TCPSocket.h"
#include "TCPShared.h"

using std::string;
using std::min;

bool TCPSocket::init() {
    this->server_addr.sin_family = AF_INET;
    this->server_addr.sin_port = htons(this->port);
    return (this->fd = socket(AF_INET, SOCK_STREAM, 0)) != -1;
}

bool TCPSocket::init_server() {
    this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return this->init() &&
        (bind(this->fd, (sockaddr*)&this->server_addr, sizeof(this->server_addr)) != -1) &&
        (listen(this->fd, 10) != -1);
}

bool TCPSocket::init_client() {
    this->server_addr.sin_addr.s_addr = inet_addr(this->server_IP);
    return this->init() &&
        connect(this->fd, (sockaddr*)&this->server_addr, sizeof(this->server_addr)) != -1;
}

bool TCPSocket::initialize() {
    return this->is_server ? this->init_server() : this->init_client();
}

int TCPSocket::accept() {
    static socklen_t len = sizeof(struct sockaddr);
    if(!this->is_server)    return -1;
    return ::accept(this->fd, (struct sockaddr*)&this->client_addr, &len);
}

const char* TCPSocket::get_client_IP() {
    return inet_ntoa(this->client_addr.sin_addr);
}

string TCPSocket::recv_large_data(int fd) {
    if(!this->is_server)    fd = this->fd;
    assert(fd != -1);

	string data;
	filepacket file;
	do {
		if(!this->recv(file, fd)) return "";
		data.append(file.content, file.len);
	} while(!file.finished);
    return data;
}

bool TCPSocket::send_large_data(const string& data, int fd) {
    if(!this->is_server)    fd = this->fd;
    assert(fd != -1);

    filepacket file;
    size_t sent = 0, trans;
    while(sent < data.size()) {
        trans = min((size_t)MAX_FILEPACK_LEN, data.size()-sent);
        memcpy(file.content, data.c_str() + sent, trans);
        sent += trans;
        file.finished = (sent == data.size());
        file.len = trans;
        if(!this->send(file, fd))   return false;
    }
    return true;
}
