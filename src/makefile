all : API client ServerGuard

tcp = include/TCPSocket.h shared/TCPSocket.cpp
udp = include/UDPSocket.h shared/UDPSocket.cpp
flags = -std=c++11 -I ./include
outdir = ../build

API : $(wildcard TaskAPI/%.cpp) $(udp) include/server.h include/headers.h
	g++ TaskAPI/APIs.cpp shared/UDPSocket.cpp -fPIC -shared -o $(outdir)/libAPI.so $(flags)
client : client_stub.cpp TaskAPI/APIs.h
	g++ client_stub.cpp -L. -lAPI -o $(outdir)/client_stub $(flags)
ServerGuard : $(wildcard ServerGuard/%.cpp) $(udp) $(tcp) include/server.h include/headers.h
	g++ ServerGuard/ServerGuard.cpp shared/UDPSocket.cpp shared/TCPSocket.cpp -o $(outdir)/ServerGuard $(flags)
TaskManager : TaskManager.cpp TCPSocket.h TCPSocket.cpp headers.h
	g++ TaskManager.cpp TCPSocket.cpp -o ../build/TaskManager -std=c++11 -lpthread
