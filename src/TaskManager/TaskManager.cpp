#include "headers.h"
#include "TCPSocket.h"
#include "TCPShared.h"
#include "terminal.h"
#include "TaskManager/Server.h"
#include "TaskManager/ObjectManager.h"

using std::string;
using OM = ObjectManager;

TCPSocket tcp;

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
    if(!server) {
        printf_msg("No available server!");
        return false;
    }
    if(!server->launch(filepath)) {
        server->busy = false;
        return false;
    } else {
        return true;
    }
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
                    printf_msg("#%-2d [%s]  online=%d  busy=%d  task=%-15s", s.id, s.IP.c_str(), s.online, s.busy, s.task_name.c_str());
                }
            }
            printf_msg("");
            auto res = OM::get_mem_keysize();
            size_t tot_size = 0;
            for(const auto& kv: res) {
                printf_msg("creator: #%-2d, size: %8luB, name: %s", 0, kv.second, kv.first.c_str());
                tot_size += kv.second;
            }
            printf_msg("total memory blocks: %d, total size %luB", res.size(), tot_size);
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
            printf_msg("Input task zip file path, or input [q] to cancel.");
            move_cursor(1, 0);
            printf_msg("");
            move_cursor(2, 0);
            printf_msg("");
            move_cursor(1, 0);
            printf("\r");
            char cmd[100];
            show_cursor();
            scanf("%s", cmd);
            hide_cursor();
            if(!launch_task(cmd)) {
                int time = 5;
                while(time--) {
                    move_cursor(3, 1);
                    printf_msg("Launching task failed. Back in %d seconds.", time+1);
                    sleep(1);
                }
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
