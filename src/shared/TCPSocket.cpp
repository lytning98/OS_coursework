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

/*  连续接收数据, 用于接收文件或数据块等
    [fd]        fd, 自动校验和设置
    [callback]  接收到数据包后的回调, 参数为 const filepacket& 类型     */
bool TCPSocket::continuous_recv(int fd, std::function<void(const filepacket&)> callback) {
    this->check_and_set_fd(fd);
    filepacket file;
    do {
        if(!this->recv(file, fd))   return false;
        callback(file);
    } while(!file.finished);
    return true;
}

string TCPSocket::recv_large_data(int fd) {
	string data;
    bool res = this->continuous_recv(fd, [&data](const filepacket& file) {
        data.append(file.content, file.len);
    });
    return res ? data : "";
}

int TCPSocket::recv_file(const string& path, int conn_fd) {
    int file_fd = open(path.c_str(), O_WRONLY | O_CREAT, 0666);
    if(file_fd < 0)
        return 1;
    bool res = this->continuous_recv(conn_fd, [file_fd](const filepacket& file) {
        write(file_fd, file.content, file.len);
    });
    ::close(file_fd);
    return res ? 0 : 2;
}

bool TCPSocket::send_large_data(const string& data, int fd) {
    this->check_and_set_fd(fd);

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

int TCPSocket::send_file(const string& path, int fd) {
    this->check_and_set_fd(fd);

    std::ifstream in(path.c_str(), std::ios::in|std::ios::binary);
    if(!in.is_open()) {
        return 1;
    }

    filepacket file;
    while(file.len = in.readsome(file.content, MAX_FILEPACK_LEN)) {
        file.finished = (in.peek() == EOF);
        if(!this->send(file, fd))   return 2;
    }

    in.close();
    return 0;
}

void TCPSocket::close() {
    shutdown(this->fd, SHUT_RDWR);
    ::close(this->fd);
}