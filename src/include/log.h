#ifndef log_H_
#define log_H_
#include <string>

namespace Log {

    void set_logfile(const char* file);

    void log(const std::string& str);

    template <typename... Args>
    void log(const char* format, Args... args) {
        char buf[128];
        sprintf(buf, format, args...);
        log(std::string(buf));
    }
}

#endif