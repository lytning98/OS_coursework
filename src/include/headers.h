#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cassert>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <thread>