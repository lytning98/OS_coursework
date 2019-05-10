all : API client ServerGuard

API : APIs.cpp APIs.h UDPSocket.cpp UDPSocket.h server.h headers.h
	g++ APIs.cpp UDPSocket.cpp -fPIC -shared -o libAPI.so -std=c++11
client : client_stub.cpp APIs.h
	g++ client_stub.cpp -L. -lAPI -o client_stub -std=c++11
ServerGuard : ServerGuard.cpp UDPSocket.h UDPSocket.cpp server.h headers.h
	g++ ServerGuard.cpp UDPSocket.cpp -o ServerGuard -std=c++11