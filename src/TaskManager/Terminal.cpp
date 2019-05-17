#include "headers.h"
#include "TaskManager/Servers.h"
#include "TaskManager/Terminal.h"
#include "TaskManager/ObjectManager.h"
#include <termio.h>

namespace Terminal {

using OM = ObjectManager;
using std::string;

inline int getch(void) {
    struct termios tm, tm_old;
    int fd = 0, ch;
    if (tcgetattr(fd, &tm) < 0) {//保存现在的终端设置
        return -1;
    }
    tm_old = tm;
    cfmakeraw(&tm);//更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
    if (tcsetattr(fd, TCSANOW, &tm) < 0) {//设置上更改之后的设置
        return -1;
    }
    ch = getchar();
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//更改设置为最初的样子
        return -1;
    }
    return ch;
}

inline int move_cursor(int x, int y) {
    printf("\033[%d;%dH", (x), (y));
}

inline int hide_cursor() {
    printf("\033[?25l");
}

inline int show_cursor() {
    printf("\033[?25h");
}


static bool _run_printing = true;
void printer() {
    int timecount = 0;
    while(true) {
        if(_run_printing) {
            move_cursor(5, 0);
            auto list = Servers::get_server_list();
            if(list.size() == 0) {
                printf_msg("No server is connected yet. (For %d seconds)", ++timecount);
            } else {
                timecount = 0;
                for(const auto& s: list) {
                    if(s.online)
                        printf_msg("#%-2d [%s]  online  busy=%d  task=%-15s", s.id, s.IP.c_str(), s.busy, s.task_name.c_str());
                    else
                        printf_msg("#%-2d [%s]  offline", s.id, s.IP.c_str());
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

void launch_CUI(int port) {
    hide_cursor();
    putchar('\n');
    printf_msg("Press [a] to launch task. Press [q] to quit.");
    printf_msg("===================");
    std::thread printer(Terminal::printer);
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
            string cmd;
            show_cursor();
            std::cin>>cmd;
            hide_cursor();
            int code = 0;
            if(cmd != "q" && (code = Servers::launch_task(cmd.c_str())) != 0) {
                switch(code) {
                    case 1 : printf_msg("No server available!"); break;
                    case 2 : printf_msg("Open file [%s] failed.", cmd.c_str()); break;
                    case 3 : printf_msg("TCP send failed."); break;
                    case 4 : printf_msg("File transfer via TCP failed."); break;
                }

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


}// end namespace
