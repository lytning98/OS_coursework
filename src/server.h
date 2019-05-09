/*
	ServerGuard 和 Task API 共用的头文件
*/

#ifndef server_H_
#define server_H_

enum msg_type {
	// ====== sender Task ======
	SET_SHM_ID,			// 设置共享内存 ID		payload : shm_id
	REQUEST_DATA,		// 请求数据				payload : data_name
	TRANS_COMMIT,		// 数据已处理完毕		payload : none
	QUIT,               // 执行结束				payload : none
	// ====== sender ServerGuard ======
	TRANS_RUNNING,		// 数据包(未结束)		payload : data_size
	TRANS_FINISHED,		// 数据包(结束)			payload : data_size
	INIT_DONE			// 共享内存初始化完成	payload : none
};

struct msgpack{
	enum msg_type type;
	msgpack(enum msg_type type = QUIT) : type(type) {}
	// payload
	union {	
		int shm_id;
		size_t data_size;
		char data_name[32];
	};
};

// 用于 UNIX 域 UDP 通信的 socket 文件
const char* SOCKET_FILE = "socketf";
// 用于在 ServerGuard 和 Task 之间交换数据的共享内存大小
const int SHM_BUF_SIZE = 10240;

#endif