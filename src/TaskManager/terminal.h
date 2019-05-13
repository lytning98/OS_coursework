#include <termio.h>

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

template<typename... Args>
void printf_msg(const char* format, Args... args) {
    char temp[100];
    sprintf(temp, format, args...);
    printf("%-50s\n\r", temp);
}

inline void printf_msg(const char* str) {
    printf("%-50s\n\r", str);
}
