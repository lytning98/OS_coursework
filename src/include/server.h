/*
	ServerGuard 和 Task API 共用的头文件
*/

#ifndef server_H_
#define server_H_

enum msg_type {
	// ====== sender Task ======
	REQUEST_DATA,		// payload : mem_name				请求有名内存区内的数据
	QUIT,               // payload : none					执行结束
	CREATE_NAMED_MEM,	// payload : mem_name + mem_size	请求创建有名内存区
	WRITE_NAMED_MEM,	// payload : shm_id + shm_size		请求写入有名内存区		
	TRANS_DONE,			// payload : none					共享内存区中的数据已读取完毕
	// ====== sender ServerGuard ======
	REQUEST_DONE,		// payload : shm_id + shm_size		请求数据已写入共享内存
	RESULTS,			// payload : errcode				操作完成返回结果
	// ====== BOTH ============
	HELLO,				// payload : none					用于设置服务器端的客户端地址
};

struct msgpack{
	enum msg_type type;
	msgpack(enum msg_type type = QUIT) : type(type) {}
	// payload
	union {
		int shm_id;
		char mem_name[32];
		int errcode;
	};
	size_t _size;
	#define mem_size _size
	#define shm_size _size
};

// 用于 UNIX 域 UDP 通信的 socket 文件
const char* SOCKET_FILE = "socketf.socket";
const char* SOCKET_CLIENT_FILE = "client.socket";
// 用于在 ServerGuard 和 Task 之间交换数据的共享内存大小
const int SHM_BUF_SIZE = 10240;

/*
-	创建、获取、卸载共享内存区
	[shm_id]	ID
	[size]		大小
*/
inline void* _get_shm_ptr(int shm_id, size_t size, int flag) {
	int hShm = shmget(shm_id, size, flag);
	if(hShm == -1)  return nullptr;
	else            return shmat(hShm, NULL, 0);
}

inline void* create_shm(int shm_id, size_t size) {
	return _get_shm_ptr(shm_id, size, IPC_CREAT | 0666);
}
inline void* get_shm(int shm_id, size_t size) {
	return _get_shm_ptr(shm_id, size, 0);
}

inline void* unmap_shm(void* shm_ptr) {
	shmdt(shm_ptr);
}
inline void* del_shm(int shm_id) {
	shmctl(shm_id, IPC_RMID, 0);
}

#endif
