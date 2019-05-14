#include "headers.h"
#include "TaskManager/ObjectManager.h"

using std::vector;
using std::pair;
using std::string;
typedef std::lock_guard<std::mutex> lock;

std::unordered_map<string, string> ObjectManager::mem_map;
std::mutex ObjectManager::M;

vector<pair<string, size_t>> ObjectManager::get_mem_keysize() {
    vector<pair<string, size_t>> ret;
    lock lk(M);
    for(const auto& kv : mem_map) {
        ret.emplace_back(kv.first, kv.second.size());
    }
    return ret;
}

bool ObjectManager::exist_mem(const char* mem_name) {
    lock lk(M);
    return mem_map.count(mem_name);
}

bool ObjectManager::create_mem(const char* mem_name, size_t size) {
    if(exist_mem(mem_name)) return false;
    lock lk(M);
    mem_map[mem_name] = string(size, 0);
    return true;
}

std::string ObjectManager::get_mem(const char* mem_name) {
    if(!exist_mem(mem_name))    return "";
    lock lk(M);
    return mem_map[mem_name];
}
