#from ctypes import *
import time
import FDFSPythonClient
import sys
import os
#libtest = cdll.LoadLibrary(os.getcwd() + '/python_test.so')
sys.path.append(os.getcwd())
# oprint libtest.fdfs_download('/etc/fdfs/client.conf', 'group2/M00/00/00/CgEIzVRJ-GGAfXv9AAkVVGMybN88065291', '~/test.jpg')
#tt = c_char_p*1
#res = (c_char_p)("gggg")
#res = create_string_buffer("ggggggg")
#g = libtest.test("gg");
# print ctyeps.sting_at(g)
res = ""
#r = FDFSPythonClient.MyTest("ls -l", res)
o = FDFSPythonClient.fdfs_init("/etc/fdfs/client.conf", 7)

#obj_handle = o
lastTime = time.time()

r = FDFSPythonClient.fdfs_delete(
    "group2", "M00/00/00/CgEIzVRlv3SAYJCTAAAsFwWtoVg9432330")

print(r)
