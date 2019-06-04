COMPILE = g++ -Wall -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -g -O1 -DDEBUG_FLAG -DOS_LINUX -DIOEVENT_USE_EPOLL
INC_PATH = -I/usr/include -I/usr/include/python3.6m
LIB_PATH = -L/usr/lib -ljsoncpp -lfastcommon -lfdfsclient
FDFS_SHARED_OBJS = FDFSPythonClient.lo FDFSClient.lo

FDFSPythonClient.so: $(FDFS_SHARED_OBJS)
	$(COMPILE) -o test/FDFSPythonClient.so -shared $(FDFS_SHARED_OBJS) $(LIB_PATH)
FDFSPythonClient.lo: FDFSPythonClient.cpp FDFSPythonClient.h
	$(COMPILE) -c -fPIC -o FDFSPythonClient.lo FDFSPythonClient.cpp $(INC_PATH)
FDFSClient.lo: FDFSClient.cpp FDFSClient.h
	$(COMPILE) -c -fPIC -o FDFSClient.lo FDFSClient.cpp $(INC_PATH)

clean:
	rm -f *.o *.lo *.so test/*.o test/*.lo test/*.so
