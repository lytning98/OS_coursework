/*
=======================
	Task API 头文件
=======================
*/
#ifndef APIs_H_
#define APIs_H_

#include "UDPSocket.h"
#include "server.h"

class API{
private:
    UDPSocket udp;
    void* shm_ptr;

    inline bool recv(msgpack& pack, enum msg_type type){
        do{
            if(!udp.recv(pack)) return false;
        }while(pack.type != type);
        return true;
    }
public:
    bool initialize();
    void quit();
    std::string request_data(const char* data_name);
};

#endif