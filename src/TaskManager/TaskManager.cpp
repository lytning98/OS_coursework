#include "headers.h"
#include "TCPSocket.h"
#include "TCPShared.h"
#include "terminal.h"
#include "TaskManager/Server.h"

TCPSocket tcp;
using std::string;

std::vector<Server> servers;

void listener() {
    while(true) {
        int fd = tcp.accept();
        servers.emplace_back(fd, tcp.get_client_IP());
    }
}

bool launch_task(const char* filepath) {
    Server* server = nullptr;
    for(auto& s : servers) {
        if(s.online && !s.busy) {
            s.busy = true;
            server = &s;
        }
    }
    if(!server) return false;
    return server->launch(filepath);
}

bool _run_printing = true;
void printer() {
    int timecount = 0;
    while(true) {
        if(_run_printing) {
            move_cursor(5, 0);
            if(servers.size() == 0) {
                printf_msg("No server is connected yet. (For %d seconds)", ++timecount);
            } else {
                timecount = 0;
                for(const auto& s: servers) {
                    printf_msg("[%s]  online=%d  busy=%d  task=%-15s", s.IP.c_str(), s.online, s.busy, s.task_name.c_str());
                }
            }
            fflush(stdout);
        }
        sleep(1);
    }
}

void CUI(int port) {
    hide_cursor();
    putchar('\n');
    printf_msg("Press [a] to launch task. Press [q] to quit.");
    printf_msg("===================");
    std::thread printer(::printer);
    while(true) {
        char ch = getch();
        if(ch == 'q') {
            system("clear");
            exit(0);
        } else if(ch == 'a') {
            _run_printing = false;
            move_cursor(3, 0);
            printf_msg("Input [q] to cancel.");
            move_cursor(1, 0);
            printf_msg("Enter info : ");
            char cmd[100];
            show_cursor();
            scanf("%s", cmd);
            hide_cursor();
            if(!launch_task(cmd)) {
                printf_msg("Launching task failed. Return in 5 seconds.");
                sleep(5);
            }
            move_cursor(1, 0);
            printf_msg("Runing on port %d.", port);
            printf_msg("");
            printf_msg("Press [a] to launch task. Press [q] to quit.");
            _run_printing = true;
        }
    }
    printer.join();
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
    CUI(port);
}
