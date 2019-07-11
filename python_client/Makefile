COMPILE = g++ -Wall -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -g -O1 -DDEBUG_FLAG -DOS_LINUX -DIOEVENT_USE_EPOLL
INC_PATH = -I/usr/include -I/usr/include/python3.6m
LIB_PATH = -L/usr/lib -ljsoncpp -lfastcommon -lfdfsclient
TARGET_LIB = /usr/local/lib/python3.6/dist-packages

FDFS_SHARED_OBJS = FDFSPythonClient.lo FDFSClient.lo

CLIENT_SHARED_LIBS = FDFSPythonClient.so

FDFSPythonClient.so: $(FDFS_SHARED_OBJS)
	$(COMPILE) -shared -o $@ $^ $(LIB_PATH)
FDFSPythonClient.lo: FDFSPythonClient.cpp FDFSPythonClient.h
	$(COMPILE) -c -fPIC -o $@ $< $(INC_PATH)
FDFSClient.lo: FDFSClient.cpp FDFSClient.h
	$(COMPILE) -c -fPIC -o $@ $< $(INC_PATH)

install: $(CLIENT_SHARED_LIBS)
	mkdir -p $(TARGET_LIB)
	cp -f $(CLIENT_SHARED_LIBS) $(TARGET_LIB)
clean:
	rm -f $(FDFS_SHARED_OBJS) $(CLIENT_SHARED_LIBS)
