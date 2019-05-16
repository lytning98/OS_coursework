/*
    Server 类的实现
*/
#include "headers.h"
#include "TCPSocket.h"
#include "TCPShared.h"
#include "terminal.h"
#include "TaskManager/Server.h"
#include "TaskManager/ObjectManager.h"

using std::string;
using OM = ObjectManager;

extern TCPSocket tcp;

int Server::id_top = 0;

void Server::create_named_mem(const packet& pack) {
    packet ret(TCPMsg::RESULTS);
    ret.errcode = (OM::create_mem(pack.mem_name, pack.mem_size) == false);
    tcp.send(ret, this->fd);
}

/*
    处理 ServerGuard 的数据请求
*/
void Server::request_data(const packet& pack) {
    packet ret(TCPMsg::RESULTS);

    if(!OM::exist_mem(pack.mem_name)) {
        ret.errcode = 1;
        tcp.send(ret, this->fd);
    } else {
        ret.errcode = 0;
        tcp.send(ret, this->fd);
        string data = OM::get_mem(pack.mem_name);
        tcp.send_large_data(data, this->fd);
    }
}

void Server::write_named_mem(const packet& pack) {
    packet ret(TCPMsg::RESULTS);

    if(OM::get_mem_size(pack.mem_name) < pack.size_towrite) {
        ret.errcode = 1;
        tcp.send(ret, this->fd);
        return;
    }
    ret.errcode = 0;
    tcp.send(ret, this->fd);
    string data = tcp.recv_large_data(this->fd);
    if(data.size() != pack.size_towrite) {
        ret.errcode = 2;
    } else {
        ret.errcode = OM::write_mem(pack.mem_name, data) ? 0 : 3;
    }
    tcp.send(ret, this->fd);
}

/*
    监听 ServerGuard 的请求
*/
void Server::watch() {
    this->IP = string(IP);

    packet pack;
    while(tcp.recv(pack, this->fd)) {
        switch(pack.type) {
            case TCPMsg::TASK_DONE :
                this->busy = false;
                this->task_name = "";
                break;
            case TCPMsg::REQUEST_DATA :
                this->request_data(pack);
                break;
            case TCPMsg::CREATE_NAMED_MEM :
                this->create_named_mem(pack);
                break;
            case TCPMsg::WRITE_NAMED_MEM :
                this->write_named_mem(pack);
                break;
        }
    }

    // recv failed : server disconnected
    this->online = false;
}

bool Server::launch(const char* filepath) {
    if(access(filepath, 0) < 0) {
        printf_msg("Open file [%s] failed.", filepath);
        return false;
    }

    packet pack;
    pack.type = TCPMsg::NEW_TASK;
    strcpy(pack.filename, filepath);
    if(!tcp.send(pack, this->fd)) {
        perror("TCP send failed.");
        return false;
    }

    this->task_name = string(filepath);
    if(tcp.send_file(filepath, this->fd)) {
        printf_msg("File transfer via TCP failed.");
        return false;
    } else {
        return true;
    }
}
