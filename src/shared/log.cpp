#include "headers.h"
#include "log.h"

using std::string;

namespace Log {
    static std::ofstream fout;
    static const char* path;

    void set_logfile(const char* file) {
        path = file;
    }

    void log(const string& str) {
        fout.open(path, std::ios::out|std::ios::app);
        fout.write((str+string("\n")).c_str(), str.length()+1);
        fout.close();
    }
}