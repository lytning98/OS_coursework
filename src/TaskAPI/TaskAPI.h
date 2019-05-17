/*
=======================
	Task API 头文件
=======================
*/
#ifndef APIs_H_
#define APIs_H_

#include <string>

namespace Task {

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
    [mem_name]     	数据区名                 
    [data, size]	指向数据的指针及数据大小
    [return]		成功(0)或错误代码
	[errcode]		1: 指定内存区不存在或过小; 2: ServerGuard 封包出错 (size 不一致); 3: TaskManager 写入出错 */
	int write_named_mem(const char* mem_name, const void* data, size_t size);
};

#endif
