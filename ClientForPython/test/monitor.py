import sys
import os
import FDFSPythonClient
import time

sys.path.append(os.getcwd())

o = FDFSPythonClient.fdfs_init("/etc/fdfs/client.conf", 7)

r = FDFSPythonClient.list_all_groups()[1]
    #g = FDFSPythonClient.list_one_group("group2")
s = FDFSPythonClient.list_storages("group1", "")[1]
    #t = FDFSPythonClient.list_storages("group2","127.0.0.1")
'''
print (time.time() - lastTime)*1000
print r
print g
print "##########################################"
print s
print "##########################################"
print t
'''
print(r)
print(s)
'''
print("##########################################")
print(s)
print("##########################################")
print(t)
'''