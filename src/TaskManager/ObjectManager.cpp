#include "headers.h"
#include "TaskManager/ObjectManager.h"

using std::vector;
using std::pair;
using std::string;
using std::mutex;
typedef std::lock_guard<std::mutex> lock;

std::unordered_map<string, string> ObjectManager::mem_map;
mutex ObjectManager::M;

vector<pair<string, size_t>> ObjectManager::get_mem_keysize() {
    vector<pair<string, size_t>> ret;
    lock lk(M);
    for(const auto& kv : mem_map) {
        ret.emplace_back(kv.first, kv.second.size());
    }
    return ret;
}

size_t ObjectManager::get_mem_size(const char* mem_name) {
    if(!exist_mem(mem_name))    return 0;
    lock lk(M);
    return mem_map[mem_name].size();
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

bool ObjectManager::write_mem(const char* mem_name, const string& data) {
    if(!exist_mem(mem_name))    return false;
    lock lk(M);
    assert(mem_map[mem_name].size() >= data.size());
    size_t diff = mem_map[mem_name].size() - data.size();
    mem_map[mem_name] = data + string(diff, 0);
    return true;
}

bool ObjectManager::save_mem_to(const char* mem_name, const char* filepath) {
    if(!exist_mem(mem_name))    return false;
    int fd = open(filepath, O_WRONLY | O_CREAT, 0666);
    if(fd < 0)                  return false;
    lock lk(M);
    const string& data = mem_map[mem_name];
    return write(fd, reinterpret_cast<const void*>(data.c_str()), data.size()) != -1;
}

bool ObjectManager::del_mem(const char* mem_name) {
    if(!exist_mem(mem_name))    return false;
    lock lk(M);
    mem_map.erase(mem_name);
    return true;
}

std::unordered_map<string, mutex>  ObjectManager::mutex_map;
mutex   ObjectManager::M_mutex;

vector<pair<string, bool>> ObjectManager::get_mutex_keystate() {
    vector<pair<string, bool>>  ret;
    lock lk(M_mutex);
    for(auto& kv : mutex_map) {
        if(kv.second.try_lock()) {
            ret.emplace_back(kv.first, true);
            kv.second.unlock();
        } else {
            ret.emplace_back(kv.first, false);
        }
    }
    return ret;
}

bool ObjectManager::exist_mutex(const char* mut_name) {
    lock lk(M_mutex);
    return mutex_map.count(mut_name);
}

bool ObjectManager::create_mutex(const char* mut_name) {
    if(exist_mutex(mut_name))   return false;
    lock lk(M_mutex);
    //  引用 map 内元素时自动构造新实例
    const auto& ignored = mutex_map[mut_name];
    return true;
}

bool ObjectManager::lock_mutex(const char* mut_name) {
    if(!exist_mutex(mut_name))  return false;
    mutex_map[mut_name].lock();
    return true;
}

bool ObjectManager::unlock_mutex(const char* mut_name) {
    if(!exist_mutex(mut_name))  return false;
    mutex_map[mut_name].unlock();
    return true;
}