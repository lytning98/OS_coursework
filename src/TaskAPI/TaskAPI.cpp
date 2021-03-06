/*
=======================
	Task API 的实现
=======================
*/
#include "headers.h"
#include "server.h"
#include "UDPSocket.h"
#include "TaskAPI.h"

using std::string;

namespace Task {

UDPSocket udp;
void* shm_ptr;

// 阻塞接收 UDP 数据包直到收到指定类型的数据包
static bool recv(msgpack& pack, UDPMsg type) {
    do{
        if(!udp.recv(pack)) return false;
    }while(pack.type != type);
    return true;
}

/* 获取当前工作路径 用于设置 Socket 服务器地址
   ServerGuard 创建的 Socket 地址为当前工作路径 */
static string get_signature() {
    char buffer[1024] = {0};
    if(!getcwd(buffer, 1024))   return "";
    if(strlen(buffer) > 90) {
        printf("Warning : current working directory path is too long!");
    }
    string ret(buffer);
    ret = ret.substr(0, ret.length()-4); // Task 进程运行于 S.G. 同目录下的 tmp/ 目录下, 因此去掉最后四个字符
    return  ret.substr(0, 90);
}

int shmid_top = 0;
bool initialize() {
    srand(time(NULL));
    shmid_top = rand();
    string sign = get_signature();
    string socket_file = string(SOCKET_FILE) + sign;
    string socket_client_file = string(SOCKET_CLIENT_FILE) + sign;
    udp = UDPSocket(socket_file.c_str(), socket_client_file.c_str());
    
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

void quit() {
    udp.send(msgpack(UDPMsg::QUIT));
}

std::string request_data(const char* mem_name) {
    msgpack pack(UDPMsg::REQUEST_DATA);
    strcpy(pack.mem_name, mem_name);
    if(!udp.send(pack)) {
        return "";
    }
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

int create_named_mem(const char* mem_name, size_t size) {
    msgpack pack(UDPMsg::CREATE_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    pack.mem_size = size;
    udp.send(pack);
    recv(pack, UDPMsg::RESULTS);
    return pack.errcode;
}

int write_named_mem(const char* mem_name, const void* data, size_t size) {
    if(size == 0) {
        throw "Size of named memory blocks must be greater than zero.";
    }
    int shm_id = ++shmid_top, shm_handle;
    void* shm_ptr = create_shm(shm_id, size, shm_handle);
    if(shm_ptr == nullptr) {
        return 4;  
    }
    memcpy(shm_ptr, data, size);
    msgpack pack(UDPMsg::WRITE_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    pack.mem_size = size;
    pack.shm_id = shm_id;
    udp.send(pack);
    recv(pack, UDPMsg::RESULTS);
    unmap_shm(shm_ptr);
    del_shm(shm_handle);
    return pack.errcode;
}

// 接受并返回 RESULTS 数据包的 errcode
static int recv_ret_results() {
    static msgpack pack;
    recv(pack, UDPMsg::RESULTS);
    return pack.errcode;
}

int del_named_mem(const char* mem_name) {
    msgpack pack(UDPMsg::DEL_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    udp.send(pack);
    recv_ret_results();
}

int create_mutex(const char* mut_name) {
    msgpack pack(UDPMsg::CREATE_MUTEX);
    strcpy(pack.mut_name, mut_name);
    udp.send(pack);
    recv_ret_results();
}

int lock_mutex(const char* mut_name) {
    msgpack pack(UDPMsg::LOCK_MUTEX);
    strcpy(pack.mut_name, mut_name);
    udp.send(pack);
    recv_ret_results();
}

int unlock_mutex(const char* mut_name) {
    msgpack pack(UDPMsg::UNLOCK_MUTEX);
    strcpy(pack.mut_name, mut_name);
    udp.send(pack);
    recv_ret_results();
}

} //end namespace