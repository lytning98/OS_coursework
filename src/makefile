.PHONY:TaskManager
all : API ServerGuard TaskManager

tcp = include/TCPSocket.h shared/TCPSocket.cpp
udp = include/UDPSocket.h shared/UDPSocket.cpp
flags = -std=c++11 -I ./include
outdir = ../build
src_ServerGuard = $(wildcard ./ServerGuard/*.cpp)
src_TaskManager = $(wildcard ./TaskManager/*.cpp)
src_TaskAPI = $(wildcard ./TaskAPI/*.cpp)
src_shared = $(wildcard ./shared/*.cpp)

API : $(src_TaskAPI) $(udp) include/server.h include/headers.h include/log.h
	g++ $(src_TaskAPI) $(src_shared) -fPIC -shared -o $(outdir)/libTaskAPI.so $(flags)
	cp $(outdir)/libTaskAPI.so /lib
ServerGuard : $(src_ServerGuard) $(udp) $(tcp) include/server.h include/headers.h include/log.h
	g++ $(src_ServerGuard) $(src_shared) -o $(outdir)/ServerGuard $(flags)
TaskManager : $(src_TaskManager) $(tcp) $(wildcard include/TaskManager/*.h) include/headers.h include/log.h
	g++ $(src_TaskManager) $(src_shared) -o $(outdir)/TaskManager -lpthread $(flags)
