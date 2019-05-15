/*
=======================
	Task API 的实现
=======================
*/
#include "headers.h"
#include "server.h"
#include "UDPSocket.h"
#include "APIs.h"

// 阻塞接收 UDP 数据包直到收到指定类型的数据包
bool API::recv(msgpack& pack, UDPMsg type) {
    do{
        if(!udp.recv(pack)) return false;
    }while(pack.type != type);
    return true;
}

bool API::initialize() {
    udp = UDPSocket(SOCKET_FILE, SOCKET_CLIENT_FILE);
    if(!udp.initialize()) {
        printf("Initializing failed.\n");
        return false;
    } else {
        errno = 0;
        if(!udp.send(msgpack(UDPMsg::HELLO))) {
            printf("Initial message sending failed.\n");
            return false;
        } else {
            return true;
        }
    }
}

void API::quit() {
    udp.send(msgpack(UDPMsg::QUIT));
}

std::string API::request_data(const char* mem_name) {
    msgpack pack(UDPMsg::REQUEST_DATA);
    strcpy(pack.mem_name, mem_name);
    udp.send(pack);
    recv(pack, UDPMsg::RESULTS);
    if(pack.errcode) {
        return "";
    }
    recv(pack, UDPMsg::REQUEST_DONE);
    void* shm_ptr = get_shm(pack.shm_id, pack.shm_size);
    std::string buffer((const char*)shm_ptr, pack.shm_size);
    unmap_shm(shm_ptr);
    udp.send(msgpack(UDPMsg::TRANS_DONE));
    return buffer;
}

int API::create_named_mem(const char* mem_name, size_t size) {
    msgpack pack(UDPMsg::CREATE_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    pack.mem_size = size;
    udp.send(pack);
    recv(pack, UDPMsg::RESULTS);
    return pack.errcode;
}

int API::write_named_mem(const char* mem_name, const void* data, size_t size) {
    if(size == 0) {
        throw "Size of named memory blocks must be greater than zero.";
    }
    int shm_id = time(NULL);
    void* shm_ptr = create_shm(shm_id, size);
    memcpy(shm_ptr, data, size);

    msgpack pack(UDPMsg::WRITE_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    pack.mem_size = size;
    pack.shm_id = shm_id;
    udp.send(pack);
    recv(pack, UDPMsg::RESULTS);
    unmap_shm(shm_ptr);
    del_shm(shm_id);
    return pack.errcode;
}
