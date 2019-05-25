#include "TaskAPI.h"

int main(){
    if(!Task::initialize()){
        perror("client error");
    }
    int errcode = Task::create_named_mem("hello", sizeof(int));
    printf("create errcode=%d\n", errcode);
    fflush(stdout);
    int res = 2019525;
    errcode = Task::write_named_mem("hello", &res, sizeof(res));   
    printf("write errcode=%d\n", errcode);
    fflush(stdout);
    std::string data = Task::request_data("hello");
    printf("len = %lu\n", data.size());
    printf("get = %d\n", *reinterpret_cast<const int*>(data.c_str()));
    fflush(stdout);
    Task::quit();
    return 0;
}
