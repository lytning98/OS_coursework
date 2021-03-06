/*
=======================
	ServerGuard 进程
=======================
*/
#include "headers.h"
#include "server.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include "TCPShared.h"

using std::string;
using std::min;

UDPSocket udp;
TCPSocket tcp;

template <typename... Args>
void systemf(const char* format, Args... args) {
	static char temp[100];
	sprintf(temp, format, args...);
	system(temp);
}

// 阻塞接收 UDP/TCP 数据包直到收到指定类型的数据包
template <typename Socket, typename Packet, typename Type>
bool recv(Socket& socket, Packet& pack, Type type) {
	do {
		if(!socket.recv(pack))	return false;
	} while(pack.type != type);
	return true;
}

/*	编译并执行 Task
	[makefile_path]		make pipe
	[exe_path]			待执行路径				*/
void make(const string& makefile_path, const string& exe_path) {
	systemf("make -f %s", makefile_path.c_str());
	systemf("./%s&", exe_path.c_str());
}

/*	解压、编译并执行 Task
	[zip_path]		zip file path				*/
void unzip_and_make(const string& zip_path) {
	systemf("unzip %s -d tmp", zip_path.c_str());
	printf("Package unziped.\n");
	chdir("tmp");
	make("makefile", "main");
	chdir("..");
}

/*
-	创建 Task 进程
*/
bool init_process(const char* filename) {
	if(access("tmp", 0) < 0) {
		mkdir("tmp", 0666);
	}

	string path = string("tmp/") + string(filename);
	int res = tcp.recv_file(path);
	if(res != 0) {
		if(res == 1)
			printf("Cannot open file [%s].\n", path.c_str());
		else
			printf("File transfer via TCP failed.");
		return false;
	}
	printf("Received task file. Saved temporarily as [%s].\n", path.c_str());

	unzip_and_make(path);
	msgpack pack;
	if(!recv(udp, pack, UDPMsg::HELLO)) {
		perror("recv hello message failed");
		return false;
	} else {
		return true;
	}
}

int shmid_top = 0;

/*
-	向 Task 传输数据
	[ptr_void]	数据缓冲区指针
	[size]		数据大小
*/
bool trans_to_task(const void* ptr_void, size_t size) {
	int shm_id = ++shmid_top, shm_handle;
	void* shm_ptr = create_shm(shm_id, size, shm_handle);
	if(shm_ptr == nullptr) {
		perror("Shared memory creating failed :");
		return false;
	}
	memcpy(shm_ptr, ptr_void, size);

	msgpack pack(UDPMsg::REQUEST_DONE);
	pack.shm_id = shm_id;
	pack.shm_size = size;
	if(!udp.send(pack)) {
		perror("send REQEUST_DONE to client error");
		return false;
	}
	recv(udp, pack, UDPMsg::TRANS_DONE);
	unmap_shm(shm_ptr);
	del_shm(shm_handle);
	return true;
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
-	处理 Task 获取数据的请求
	[pack]	type 为 REQUEST_DATA 的 msgpack (payload 中含有共享内存区名信息)
*/
void handle_request_data(const msgpack& _pack) {
	packet tcp_pack;
	msgpack pack;

	tcp_pack.type = TCPMsg::REQUEST_DATA;
	strcpy(tcp_pack.mem_name, _pack.mem_name);
	tcp.send(tcp_pack);

	recv(tcp, tcp_pack, TCPMsg::RESULTS);
	pack.type = UDPMsg::RESULTS;
	pack.errcode = tcp_pack.errcode;
	udp.send(pack);
	if(tcp_pack.errcode) {
		return;
	}
	
	string data = tcp.recv_large_data();
	trans_to_task(data.c_str(), data.size());
}

/*
-	处理 Task 写入内存区的请求
	[pack]	type 为 WRITE_NAMED_MEM 的 msgpack (payload 中含有内存区名,大小,共享内存 ID 信息)
*/
void handle_write_named_mem(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::WRITE_NAMED_MEM);
	strcpy(tcp_pack.mem_name, _pack.mem_name);
	tcp_pack.size_towrite = _pack.mem_size;
	tcp.send(tcp_pack);	// 发送请求尝试写入
	recv(tcp, tcp_pack, TCPMsg::RESULTS);

	msgpack pack(UDPMsg::RESULTS);
	if(tcp_pack.errcode) {
		pack.errcode = tcp_pack.errcode;
	} else {
		string data = trans_from_task(_pack);
		tcp.send_large_data(data);
		recv(tcp, tcp_pack, TCPMsg::RESULTS);
		pack.errcode = tcp_pack.errcode;
	}
	udp.send(pack);
}

// 接受 TaskManager 返回的操作结果并转发给 Task
void forward_results() {
	packet tcp_pack;
	recv(tcp, tcp_pack, TCPMsg::RESULTS);
	msgpack pack(UDPMsg::RESULTS);
	pack.errcode = tcp_pack.errcode;
	udp.send(pack);
}

/*
-	处理 Task 创建内存区的请求
	[pack]	type 为 CREATE_NAMED_MEM 的 msgpack (payload 中含有内存区名和大小信息)
*/
void handle_create_named_mem(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::CREATE_NAMED_MEM);
	strcpy(tcp_pack.mem_name, _pack.mem_name);
	tcp_pack.mem_size = _pack.mem_size;
	tcp.send(tcp_pack);
	forward_results();
}

/*
-	处理 Task 删除内存区的请求
	[pack]	type 为 DEL_NAMED_MEM 的 msgpack (payload 中含有内存区名信息)
*/
void handle_del_named_mem(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::DEL_NAMED_MEM);
	strcpy(tcp_pack.mem_name, _pack.mem_name);
	tcp.send(tcp_pack);
	forward_results();
}

/*
-	处理 Task 创建互斥锁的请求
	[pack]	type 为 CREATE_MUTEX 的 msgpack (payload 中含有互斥锁名信息)
*/
void handle_create_mutex(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::CREATE_MUTEX);
	strcpy(tcp_pack.mut_name, _pack.mut_name);
	tcp.send(tcp_pack);
	forward_results();
}

/*
-	处理 Task 获取互斥锁的请求
	[pack]	type 为 LOCK_MUTEX 的 msgpack (payload 中含有互斥锁名信息)
*/
void handle_lock_mutex(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::LOCK_MUTEX);
	strcpy(tcp_pack.mut_name, _pack.mut_name);
	tcp.send(tcp_pack);
	forward_results();
}

/*
-	处理 Task 解锁互斥锁的请求
	[pack]	type 为 UNLOCK_MUTEX 的 msgpack (payload 中含有互斥锁名信息)
*/
void handle_unlock_mutex(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::UNLOCK_MUTEX);
	strcpy(tcp_pack.mut_name, _pack.mut_name);
	tcp.send(tcp_pack);
	forward_results();
}

// 监控 Task 进程的主过程, 处理请求
bool watch_process(){
	msgpack pack;

	while(udp.recv(pack)){
		switch(pack.type){
			case UDPMsg::QUIT:
				tcp.send(packet(TCPMsg::TASK_DONE));
				// system("rm -rf tmp");
				return true;
			case UDPMsg::REQUEST_DATA:
				handle_request_data(pack);
				break;
			case UDPMsg::CREATE_NAMED_MEM:
				handle_create_named_mem(pack);
				break;
			case UDPMsg::WRITE_NAMED_MEM:
				handle_write_named_mem(pack);
				break;
			case UDPMsg::DEL_NAMED_MEM:
				handle_del_named_mem(pack);
				break;
			case UDPMsg::CREATE_MUTEX:
				handle_create_mutex(pack);
				break;
			case UDPMsg::LOCK_MUTEX:
				handle_lock_mutex(pack);
				break;
			case UDPMsg::UNLOCK_MUTEX:
				handle_unlock_mutex(pack);
				break;
		}
	}

	return false;
}

// 监控 TaskManager 的主过程, 处理请求
void watch_manager() {
	packet pack;
	while(tcp.recv(pack)) {
		switch(pack.type) {
			case TCPMsg::NEW_TASK:
				printf("Assigned new task : %s\n", pack.filename);
				if(init_process(pack.filename)) {
					watch_process();
					printf("Task %s done.\n", pack.filename);
				} else {
					printf("Process initializing failed.\n");
				}
				break;
		}
	}

	// tcp recv failed : server shutdown
	printf("Server has shut down the connection.\n");
}

string get_signature() {
    char buffer[1024] = {0};
    if(!getcwd(buffer, 1024))   return "";
    if(strlen(buffer) > 90) {
        printf("Warning : current working directory path is too long!");
    }
    string ret(buffer);
    return  ret.substr(0, 90);
}

int main(int argc, char** argv){
	srand(time(NULL));
	shmid_top = rand();

	if(argc == 3) {
		tcp = TCPSocket(argv[1], atoi(argv[2]));
	} else {
		printf("TaskManager address is not specified. Running in local mode (127.0.0.1:9412).\n");
		tcp = TCPSocket("127.0.0.1", 9412);
	}

	string socket_file = string(SOCKET_FILE) + get_signature();
	udp = UDPSocket(socket_file.c_str());

	if(!udp.initialize()) {
		perror("UDP Socket initializing failed");
		return -1;
	}
	if(!tcp.initialize()) {
		perror("TCP Socket initializing failed");
		return -1;
	}
	printf("Connection established.\n");
	watch_manager();
	return 0;
}
