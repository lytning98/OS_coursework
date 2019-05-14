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
using std::min;
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
        filepacket file;
        size_t sent = 0, trans;
        while(sent < data.size()) {
            trans = min((size_t)MAX_FILEPACK_LEN, data.size()-sent);
            memcpy(file.content, data.c_str() + sent, trans);
            sent += trans;
            file.finished = (sent == data.size());
            file.len = trans;
            tcp.send(file, this->fd);
        }
    }
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
        }
    }
}

bool Server::launch(const char* filepath) {
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if(!in.is_open()) {
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

    filepacket file_pack;
    while(file_pack.len = in.readsome(file_pack.content, MAX_FILEPACK_LEN)) {
        if(in.peek() == EOF)    file_pack.finished = true;
        else    file_pack.finished = false;
        tcp.send(file_pack, this->fd);
    }
    return true;
}
