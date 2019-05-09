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

/*
-	编译并执行 Task
	[makefile_path]	make pipe
	[exe_path]		待执行路径
*/
void make(const string& makefile_path, const string& exe_path){
	string cmd = "make -f " + makefile_path;
	system(cmd.c_str());
	cmd = "./" + exe_path + "&";
	system(cmd.c_str());
}

/*
-	创建共享内存区
	[shm_id]	ID
	[size]		大小
*/
void* create_shm(int shm_id, size_t size){
	int hShm = shmget(shm_id, size, IPC_CREAT | 0666);
	if(hShm == -1)  return nullptr;
	else            return shmat(hShm, NULL, 0);
}
void* shm_ptr = nullptr;

/*
-	创建 Task 进程
*/
bool init_process(){
	if(!udp.initialize())
		return false;
	msgpack pack;

	make("makefile_stub.txt", "client_stub");
	if(!udp.recv(pack) || pack.type != SET_SHM_ID){
		return false;
	}else{
		shm_ptr = create_shm(pack.shm_id, SHM_BUF_SIZE);
		udp.send(msgpack(INIT_DONE));
		return shm_ptr != nullptr;
	}
}

/*
-	向 Task 传输数据
	[ptr_void]	数据缓冲区指针
	[size]		数据大小
*/
void transfer_data(void* ptr_void, size_t size){
	msgpack pack;
	char* ptr = (char*)ptr_void;

	size_t rem = size, once;
	while(rem){
		pack.data_size = min((size_t)SHM_BUF_SIZE, rem);
		memcpy(shm_ptr, ptr, pack.data_size);
		rem -= pack.data_size;
		ptr += pack.data_size;
		pack.type = rem ? TRANS_RUNNING : TRANS_FINISHED;
		udp.send(pack);
		while(pack.type != TRANS_COMMIT)	udp.recv(pack);
	}
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
	transfer_data(res, sizeof(res));
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
