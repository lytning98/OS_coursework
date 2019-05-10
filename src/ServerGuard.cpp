/*
=======================
	ServerGuard 进程
=======================
*/
#include "headers.h"
#include "server.h"
#include "UDPSocket.h"

using std::string;
using std::min;

UDPSocket udp(SOCKET_FILE);

// 阻塞接收 UDP 数据包直到收到指定类型的数据包
bool recv(msgpack& pack, enum msg_type type) {
    do{
        if(!udp.recv(pack)) return false;
    }while(pack.type != type);
    return true;
}

/*
-	编译并执行 Task
	[makefile_path]	make pipe
	[exe_path]		待执行路径
*/
void make(const string& makefile_path, const string& exe_path) {
	string cmd = "make -f " + makefile_path;
	system(cmd.c_str());
	cmd = "./" + exe_path + "&";
	system(cmd.c_str());
}

/*
-	创建 Task 进程
*/
bool init_process() {
	if(!udp.initialize())
		return false;
	msgpack pack;

	make("makefile_stub.txt", "client_stub");
	if(!recv(pack, HELLO)) {
		return false;
	} else {
		return true;
	}
}

/*
-	向 Task 传输数据
	[ptr_void]	数据缓冲区指针
	[size]		数据大小
*/
void trans_to_task(void* ptr_void, size_t size) {
	int shm_id = time(NULL);
	void* shm_ptr = create_shm(shm_id, size);
	memcpy(shm_ptr, ptr_void, size);

	msgpack pack(REQUEST_DONE);
	pack.shm_id = shm_id;
	pack.shm_size = size;
	udp.send(pack);
	recv(pack, TRANS_DONE);
	unmap_shm(shm_ptr);
	del_shm(shm_id);
}

/*
-	从 Task 指定的共享内存区读取数据
	[pack]	type 为 WRITE_NAMED_MEM 的 msgpack (payload 中含有共享内存区 ID 和大小信息)
*/
string trans_from_task(const msgpack& pack) {
	void* shm_ptr = get_shm(pack.shm_id, pack.mem_size);
	string data((const char*)shm_ptr, pack.mem_size);
	unmap_shm(shm_ptr);
	return data;
}

/*
-	测试数据传输功能的桩代码
	**debug**
*/
void stub_transfer_test(){
	int res[4000], cksum = 0;
	for(int i = 0; i < 4000; i++)	cksum ^= (res[i] = rand());
	printf("[server]checksum %d\n", cksum);
	printf("[server]size %ld\n", sizeof(res));
	trans_to_task(res, sizeof(res));
}

/*
-	监控 Task 进程的主过程, 处理请求
*/
bool watch_process(){
	msgpack pack;

	while(udp.recv(pack)){
		switch(pack.type){
			case QUIT:
				return true;
			case REQUEST_DATA:
				stub_transfer_test();
				break;
			case CREATE_NAMED_MEM:
				break;
			case WRITE_NAMED_MEM:
				break;
		}
	}

	return false;
}

int main(int argc, char** argv){
	/*
		TCP sth
	*/
   if(!init_process() || !watch_process()){
	   perror("server ERROR");
   }
}
