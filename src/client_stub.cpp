#include "headers.h"
#include "APIs.h"

API api;

int main(){
    if(!api.initialize()){
        perror("client error");
    }
    const int* res = (const int*)api.request_data("hello").c_str();
    // printf("[clint]size %ld\n", size);
    int cksum = 0;
    for(int i = 0; i < 4000; i++)
        cksum ^= res[i];
    printf("[client]checksum %d\n", cksum);
    api.quit();
    return 0;
}
