/*
    TaskManager 与 ServerGuard 之间进行 TCP 通信的共用声明
*/
#ifndef TCPShared_H_
#define TCPShared_H_

// 描述 ServerGuard 与 TaskManager 之间进行 TCP 通信的数据包类型
enum class TCPMsg {
	// ====== sender ServerGuard ======
    REQUEST_DATA,       // payload : mem_name
    TASK_DONE,          // payload : none
    CREATE_NAMED_MEM,   // payload : mem_name + mem_size
    WRITE_NAMED_MEM,    // payload : mem_name + size_towirte
    DEL_NAMED_MEM,      // payload : mem_name
    // ====== sender TaskManager ======
    NEW_TASK,           // payload : none;
    RESULTS,            // payload : errcode
};

// ServerGuard 与 Task 之间进行 TCP 通信的数据包
struct packet {
    TCPMsg type;
    packet() {}
    packet(TCPMsg type) : type(type) {}
    // payload
    union {
        char filename[128];
        struct {
            char mem_name[64];
            union {
                size_t mem_size;
                size_t size_towrite;
            };
        };
        int errcode;
    };
};

// filepacket 的最大 payload 长度
const int MAX_FILEPACK_LEN = 10240;

// 用于发送文件或较大数据块的数据包结构
struct filepacket {
    bool finished;
    size_t len;
    char content[MAX_FILEPACK_LEN];
};

#endif