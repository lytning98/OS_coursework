/*
=======================
	Task API 头文件
=======================
*/
#ifndef APIs_H_
#define APIs_H_

#include "UDPSocket.h"
#include "server.h"

class API {
private:
	UDPSocket udp;
	void* shm_ptr;
	bool recv(msgpack& pack, UDPMsg type);

public:
	// 初始化：与 ServerGuard 的通信, 初始化共享内存
	bool initialize();
	// 通知 ServerGuard 运行完毕
	void quit();

	/*	通过 ServerGuard 向 TaskManager 请求数据
		[data_name]     数据区名                */
	std::string request_data(const char* mem_name);
	
	/*	通过 ServerGuard 创建有名内存区
		[mem_name]     数据区名
		[return]		成功(0)或错误代码		
		[errcode]		1: 指定名称已存在		*/
	int create_named_mem(const char* mem_name, size_t size);

	/*	通过 ServerGuard 向有名内存区写入数据
		[mem_name]     数据区名                 
		[data]         数据
		[return]		成功(0)或错误代码		*/
	int write_named_mem(const char* mem_name, std::string data);
};

#endif
