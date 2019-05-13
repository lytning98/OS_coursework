/*
    TaskManager 与 ServerGuard 之间进行 TCP 通信的共用声明
*/
#ifndef TCPShared_H_
#define TCPShared_H_

enum class TCPMsg {
	// ====== sender ServerGuard ======
    REQUEST_DATA,       // payload : 
    TASK_DONE,          // payload : none
    CREATE_NAMED_MEM,   // payload : mem_name + mem_size
    // ====== sender TaskManager ======
    NEW_TASK,           // payload : none;
    RESULTS,            // payload : errcode
};

struct packet {
    TCPMsg type;
    packet() {}
    packet(TCPMsg type) : type(type) {}
    // payload
    union {
        char filename[128];
        struct {
            char mem_name[64];
            size_t mem_size;
        };
        int errcode;
    };
};

const int MAX_FILEPACK_LEN = 10240;

struct filepacket {
    bool finished;
    size_t len;
    char content[MAX_FILEPACK_LEN];
};

#endif