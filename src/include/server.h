/*
	ServerGuard 和 Task API 共用的头文件
*/

#ifndef server_H_
#define server_H_

// 描述 ServerGuard 与 Task 之间进行 UDP 通信的数据包类型
enum class UDPMsg {
	// ====== sender Task ======
	REQUEST_DATA,		// payload : mem_name                       请求有名内存区内的数据
	QUIT,               // payload : none							执行结束
	CREATE_NAMED_MEM,	// payload : mem_name + mem_size			请求创建有名内存区
	WRITE_NAMED_MEM,	// payload : mem_name + shm_id + shm_size	请求写入有名内存区		
	DEL_NAMED_MEM,		// payload : mem_name						请求删除有名内存区
	TRANS_DONE,			// payload : none							共享内存区中的数据已读取完毕
	CREATE_MUTEX,		// payload : mut_name						请求创建互斥锁
	LOCK_MUTEX,			// payload : mut_name						请求获取互斥锁
	UNLOCK_MUTEX,		// payload : mut_name						请求解锁互斥锁
	// ====== sender ServerGuard ======
	REQUEST_DONE,		// payload : shm_id + shm_size		请求数据已写入共享内存
	RESULTS,			// payload : errcode				操作完成返回结果
	// ====== BOTH ============
	HELLO,				// payload : none					用于设置服务器端的客户端地址
};

// ServerGuard 与 Task 之间进行 UDP 通信的数据包
struct msgpack{
	UDPMsg type;
	msgpack(UDPMsg type = UDPMsg::QUIT) : type(type) {}
	// payload
	int shm_id;
	union {
		char mem_name[64];
		char mut_name[64];
	};
	int errcode;
	union {
		size_t mem_size;
		size_t shm_size;
	};
};

// 用于 UNIX 域 UDP 通信的 server domain file 路径
const char* SOCKET_FILE = "/task.socket";
// 用于 UNIX 域 UDP 通信的 client domain file 路径
const char* SOCKET_CLIENT_FILE = "/taskc.socket";

// 用于在 ServerGuard 和 Task 之间交换数据的共享内存大小
const int SHM_BUF_SIZE = 10240;

// 共享内存操作的辅助函数
inline void* _get_shm_ptr(int shm_id, size_t size, int flag) {
	int hShm = shmget(shm_id, size, flag);
	if(hShm == -1)  return nullptr;
	else            return shmat(hShm, NULL, 0);
}

/*	创建共享内存区
	[shm_id, size]	ID 和大小							*/
inline void* create_shm(int shm_id, size_t size) {
	return _get_shm_ptr(shm_id, size, IPC_CREAT | 0666);
}

/*	获取共享内存区
	[shm_id, size]	ID 和大小							*/
inline void* get_shm(int shm_id, size_t size) {
	return _get_shm_ptr(shm_id, size, 0);
}

/*	卸载共享内存区
	[shm_ptr]	指向共享内存区的指针                     */
inline void* unmap_shm(void* shm_ptr) {
	shmdt(shm_ptr);
}

/*	删除共享内存区
	[shm_id]	ID                                     */
inline void* del_shm(int shm_id) {
	shmctl(shm_id, IPC_RMID, 0);
}

#endif
