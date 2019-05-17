#ifndef Terminal_H_
#define Terminal_H_

#include "headers.h"

namespace Terminal {
    /*  Launch Character User Interface
        [port]  服务器端口号, 仅用于显示信息          */
    void launch_CUI(int port);

    // 按 TaskManager CUI 的格式包装 printf
    template<typename... Args>
    void printf_msg(const char* format, Args... args) {
        char temp[100];
        sprintf(temp, format, args...);
        printf("%-90s\n\r", temp);
    }

    // 按 TaskManager CUI 的格式输出字符串
    inline void printf_msg(const char* str) {
        printf("%-90s\n\r", str);
    }
}

#endif