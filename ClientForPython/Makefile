.SUFFIXES: .c .o

COMPILE = g++ -Wall -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -g -O -DDEBUG_FLAG -DOS_LINUX -DIOEVENT_USE_EPOLL
INC_PATH = -I. -I../../tracker -I../../storage -I../ -I ../../common -I/usr/local/include -I/usr/local/include/python2.7 -I/usr/local/include/fastcommon -I/usr/local/include/fdfsdfst
LIB_PATH = -L/usr/local/lib -L../ -lfastcommon  -lfdfsclient  -lpthread -ldl -rdynamic -ljsonlib
FDFS_SHARED_OBJS = FDFSPythonClient.o FDFSClient.o

FDFSPythonClient.so: $(FDFS_SHARED_OBJS)
	$(COMPILE) -shared $(LIB_PATH) -o FDFSPythonClient.so $(FDFS_SHARED_OBJS)
FDFSPythonClient.o:FDFSPythonClient.cpp FDFSPythonClient.h
	$(COMPILE) -c -fPIC FDFSPythonClient.cpp $(INC_PATH) $(LIB_PATH)
FDFSClient.o:FDFSClient.cpp FDFSClient.h
	$(COMPILE) -c -fPIC FDFSClient.cpp $(INC_PATH) $(LIB_PATH)

clean:
	rm -rf *.o *.so
