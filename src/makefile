.PHONY : all API ServerGuard TaskManager clean cleand

all : API ServerGuard TaskManager clean

flags = -std=c++11 -I ./include -fPIC -lpthread
outdir = ../build

src_ServerGuard = $(wildcard ServerGuard/*.cpp)
src_TaskManager = $(wildcard TaskManager/*.cpp)
src_TaskAPI = $(wildcard TaskAPI/*.cpp)
src_shared = $(wildcard shared/*.cpp)
sources = $(src_shared) $(src_TaskAPI) $(src_ServerGuard) $(src_TaskManager) 
dependencies = $(sources:.cpp=.d)
objects = $(sources:.cpp=.o)

$(dependencies) : %.d : %.cpp
	@set -e; rm -f $@; \
	g++ -MM $(flags) $< >> $@; \
	sed 's,\($*\)\.o[ :]*,\1.o : $@ ,g' < $@ > $@.$$$$; \
	sed 's,$(*F).o,$*.o,g' < $@.$$$$ > $@; \
	echo '	@echo "Compiling [$(*F).o] ..." '>> $@; \
	echo '	@g++ -c $< -o $(@:.d=.o) $(flags)' >> $@; \
	rm -f $@.$$$$
include $(dependencies)

API : $(src_TaskAPI:.cpp=.o) $(src_shared:.cpp=.o)
	@echo 'Linking [TaskAPI] ...'; \
	g++ $^ -fPIC -shared -o $(outdir)/libTaskAPI.so $(flags); \
	echo 'Copying shared object [libTaskAPI.so] to /lib ...'; \
	cp $(outdir)/libTaskAPI.so /lib

ServerGuard : $(src_ServerGuard:.cpp=.o) $(src_shared:.cpp=.o)
	@echo 'Linking [ServerGuard] ...'; \
	g++ $^ -o $(outdir)/ServerGuard $(flags)

TaskManager : $(src_TaskManager:.cpp=.o) $(src_shared:.cpp=.o)
	@echo 'Linking [TaskManager] ...'; \
	g++ $^ -o $(outdir)/TaskManager $(flags)

clean :
	@echo 'Cleaning object files ...'; \
	rm -f $(objects)

cleand:
	rm -f $(dependencies)
