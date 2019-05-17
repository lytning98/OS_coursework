#include "headers.h"
#include "TCPSocket.h"
#include "TCPShared.h"
#include "TaskManager/Terminal.h"
#include "TaskManager/Servers.h"

using std::string;

TCPSocket tcp;

void listener() {
    int fd;
    while((fd = tcp.accept()) != -1) {
        Servers::add_server(fd, tcp.get_client_IP());
    }
}

int main(int argc, char** argv) {
    system("clear");
    int port;
    if(argc != 2) {
        printf("Port is not specified. Using default value (9412).\n");
        tcp = TCPSocket(9412);
        port = 9412;
    } else {
        printf("Port specified : %d.\n", atoi(argv[1]));
        tcp = TCPSocket(atoi(argv[1]));
        port = atoi(argv[1]);
    }
	if(!tcp.initialize()) {
		perror("TCP Socket initializing failed : ");
		return -1;
	}
    std::thread listener(::listener);
    Terminal::launch_CUI(port);

    tcp.close();
    listener.join();
    Servers::disconnect_all();
}
