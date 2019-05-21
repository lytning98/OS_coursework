#include "TaskManager/Server.h"
#include "TaskManager/Servers.h"
#include "headers.h"

namespace Servers {

using std::vector;

static vector<Server> servers;
static std::mutex M;
typedef std::lock_guard<std::mutex> lock;

void add_server(int fd, const char* IP) {
    lock lk(M);
    servers.emplace_back(fd, IP);
}

int launch_task(const char* filepath) {
    Server* server = nullptr;
    M.lock();
    for(auto& s : servers) {
        if(s.online && !s.busy) {
            s.busy = true;
            server = &s;
            break;
        }
    }
    M.unlock();
    int code = 0;
    if(!server || (code = server->launch(filepath)) != 0) {
        lock lk(M);
        if(code) {  // launch failed
            server->busy = false;
            return code;
        } else {    // No available server
            return 1;
        }
    } else {
        return 0;
    }
}

vector<server_info> get_server_list() {
    vector<server_info> ret;
    lock lk(M);
    for(const auto& s : servers) {
        ret.emplace_back(s.id, s.online, s.busy, s.IP, s.task_name);
    }
    return ret;
}

void disconnect_all() {
    lock lk(M);
    for(auto& s : servers) {
        s.disconnect();
    }
}

} // end namespace
