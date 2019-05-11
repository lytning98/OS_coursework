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

/*
-	API: 初始化：与 ServerGuard 的通信
*/
bool API::initialize() {
    udp = UDPSocket(SOCKET_FILE, SOCKET_CLIENT_FILE);
    if(!udp.initialize() || !udp.send(msgpack(UDPMsg::HELLO)))
        return false;
    else
        return true;
}

/*
-	API：通知 ServerGuard 运行完毕
*/
void API::quit() {
    udp.send(msgpack(UDPMsg::QUIT));
}

/*
-	API：通过 ServerGuard 向 TaskManager 请求数据
    [data_name]     ID
*/
std::string API::request_data(const char* mem_name) {
    msgpack pack(UDPMsg::REQUEST_DATA);
    strcpy(pack.mem_name, mem_name);
    udp.send(pack);
    recv(pack, UDPMsg::REQUEST_DONE);
    void* shm_ptr = get_shm(pack.shm_id, pack.shm_size);
    std::string buffer((const char*)shm_ptr, pack.shm_size);
    unmap_shm(shm_ptr);
    udp.send(msgpack(UDPMsg::TRANS_DONE));
    return buffer;
}

/*
-	通过 ServerGuard 创建有名内存区
	[mem_name]     数据区名
	[return]		成功(0)或错误代码
*/
int API::create_named_mem(const char* mem_name, size_t size) {
    msgpack pack(UDPMsg::CREATE_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    udp.send(pack);
    recv(pack, UDPMsg::RESULTS);
    return pack.errcode;
}

/*	
-   通过 ServerGuard 向有名内存区写入数据
    [mem_name]     数据区名                 
    [data]         数据
    [return]		成功(0)或错误代码
*/
int API::write_named_mem(const char* mem_name, std::string data) {
    int shm_id = time(NULL);
    void* shm_ptr = create_shm(shm_id, data.size());
    memcpy(shm_ptr, (const void*)data.c_str(), data.size());

    msgpack pack(UDPMsg::WRITE_NAMED_MEM);
    strcpy(pack.mem_name, mem_name);
    pack.mem_size = data.size();
    pack.shm_id = shm_id;
    udp.send(pack);

    recv(pack, UDPMsg::RESULTS);
    unmap_shm(shm_ptr);
    del_shm(shm_id);
    return pack.errcode;
}
