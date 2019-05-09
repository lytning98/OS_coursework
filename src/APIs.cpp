/*
=======================
	Task API 的实现
=======================
*/
#include "headers.h"
#include "server.h"
#include "UDPSocket.h"
#include "APIs.h"

/*
-	初始化：与 ServerGuard 的通信, 初始化共享内存
*/
bool API::initialize(){
    udp = UDPSocket(SOCKET_FILE, "clientfile");

    msgpack pack(SET_SHM_ID);
    int shm_id = time(NULL);
    pack.shm_id = shm_id;

    if(!(udp.initialize() && udp.send(pack) && recv(pack, INIT_DONE)))
        return false;
    int hShm = shmget(shm_id, SHM_BUF_SIZE, 0);
    if(hShm == -1)
        return false;
    else
        shm_ptr = shmat(hShm, nullptr, 0);
    return true;
}

/*
-	API：通过 ServerGuard 向 TaskManager 请求数据
    [data_name]     ID
*/
std::string API::request_data(const char* data_name){
    msgpack pack(REQUEST_DATA);
    strcpy(pack.data_name, data_name);
    udp.send(pack);
    std::string buffer;
    do {
        udp.recv(pack);
        buffer.append((const char*)shm_ptr, pack.data_size);
        udp.send(msgpack(TRANS_COMMIT));
    } while(pack.type != TRANS_FINISHED);
    return buffer;
}

/*
-	API：通知 ServerGuard 运行完毕
*/
void API::quit(){
    udp.send(msgpack(QUIT));
}