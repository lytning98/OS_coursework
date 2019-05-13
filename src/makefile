.PHONY:TaskManager
all : API client ServerGuard TaskManager

tcp = include/TCPSocket.h shared/TCPSocket.cpp
udp = include/UDPSocket.h shared/UDPSocket.cpp
flags = -std=c++11 -I ./include
outdir = ../build
src_ServerGuard = $(wildcard ./ServerGuard/*.cpp)
src_TaskManager = $(wildcard ./TaskManager/*.cpp)

API : $(wildcard TaskAPI/%.cpp) $(udp) include/server.h include/headers.h
	g++ TaskAPI/APIs.cpp shared/UDPSocket.cpp -fPIC -shared -o $(outdir)/libTaskAPI.so $(flags)
	cp $(outdir)/libTaskAPI.so /lib
client : client_stub.cpp TaskAPI/APIs.h
	g++ client_stub.cpp -lTaskAPI -o $(outdir)/client_stub $(flags)
ServerGuard : $(src_ServerGuard) $(udp) $(tcp) include/server.h include/headers.h
	g++ $(src_ServerGuard) shared/UDPSocket.cpp shared/TCPSocket.cpp -o $(outdir)/ServerGuard $(flags)
TaskManager : $(src_TaskManager) $(tcp) $(wildcard include/TaskManager/*.h) include/headers.h
	g++ $(src_TaskManager) shared/TCPSocket.cpp -o $(outdir)/TaskManager -lpthread $(flags)
