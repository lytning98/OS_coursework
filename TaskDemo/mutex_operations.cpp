#include "TaskAPI.h"
#include "unistd.h"

int main(){
    if(!Task::initialize()){
        perror("client error");
    }
    int errcode = Task::create_mutex("hello");
    printf("create errcode=%d\n", errcode);
    errcode = Task::lock_mutex("hello");
    printf("lock errcode=%d\ninput a char:", errcode);
    sleep(5); 
    errcode = Task::unlock_mutex("hello");
    printf("unlock errcode=%d\n", errcode);
    Task::quit();
    return 0;
}
