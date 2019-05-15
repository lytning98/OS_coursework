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

UDPSocket udp(SOCKET_FILE);
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
bool init_process(const char* filename) {
	if(access("tmp", 0) < 0) {
		mkdir("tmp", 0666);
	}
	string path = string("tmp/") + string(filename);
	int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0666);
	if(fd < 0) {
		printf("Cannot open file [%s].\n", path.c_str());
		return false;
	}
	filepacket data;
	do {
		if(!tcp.recv(data))	return false;
		write(fd, data.content, data.len);
	} while(!data.finished);
	close(fd);
	printf("Received task file. Saved temporarily as [%s].\n", path.c_str());
	systemf("unzip %s -d tmp", path.c_str());
	printf("Package unziped.\n");
	chdir("tmp");
	make("makefile", "client_stub");
	chdir("..");
	msgpack pack;
	if(!recv(udp, pack, UDPMsg::HELLO)) {
		perror("recv hello message failed");
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
bool trans_to_task(const void* ptr_void, size_t size) {
	int shm_id = time(NULL);
	void* shm_ptr = create_shm(shm_id, size);
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
	del_shm(shm_id);
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
-	处理 Task 创建内存区的请求
	[pack]	type 为 CREATE_NAMED_MEM 的 msgpack (payload 中含有内存区名和大小信息)
*/
void handle_create_named_mem(const msgpack& _pack) {
	packet tcp_pack(TCPMsg::CREATE_NAMED_MEM);
	strcpy(tcp_pack.mem_name, _pack.mem_name);
	tcp_pack.mem_size = _pack.mem_size;
	tcp.send(tcp_pack);
	recv(tcp, tcp_pack, TCPMsg::RESULTS);

	msgpack pack(UDPMsg::RESULTS);
	pack.errcode = tcp_pack.errcode;
	udp.send(pack);
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

/*
-	监控 Task 进程的主过程, 处理请求
*/
bool watch_process(){
	msgpack pack;

	while(udp.recv(pack)){
		switch(pack.type){
			case UDPMsg::QUIT:
				tcp.send(packet(TCPMsg::TASK_DONE));
				system("rm -rf tmp");
				return true;
			case UDPMsg::REQUEST_DATA:
				handle_request_data(pack);
				// stub_transfer_test();
				break;
			case UDPMsg::CREATE_NAMED_MEM:
				handle_create_named_mem(pack);
				break;
			case UDPMsg::WRITE_NAMED_MEM:
				handle_write_named_mem(pack);
				break;
		}
	}

	return false;
}

void watch_manager() {
	packet pack;
	while(true) {
		tcp.recv(pack);
		switch(pack.type) {
			case TCPMsg::NEW_TASK:
				printf("Assigned new task : %s\n", pack.filename);
				init_process(pack.filename);
				watch_process();
				printf("Task %s done.\n", pack.filename);
				break;
		}
	}
}

bool stub_init_process() {
	msgpack pack;

	make("makefile", "client_stub");
	if(!recv(udp, pack, UDPMsg::HELLO)) {
		return false;
	} else {
		return true;
	}
}

int main(int argc, char** argv){
	if(argc == 3) {
		tcp = TCPSocket(argv[1], atoi(argv[2]));
	} else {
		printf("TaskManager address is not specified. Running in local mode (127.0.0.1:9412).\n");
		tcp = TCPSocket("127.0.0.1", 9412);
	}

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
