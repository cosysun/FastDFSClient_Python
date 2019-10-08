#from ctypes import *
import FDFSPythonClient as fdfs
import time
import sys
# import os
#libtest = cdll.LoadLibrary(os.getcwd() + '/python_test.so')
# sys.path.append(os.getcwd())
# oprint libtest.fdfs_download('/etc/fdfs/client.conf', 'group2/M00/00/00/CgEIzVRJ-GGAfXv9AAkVVGMybN88065291', '~/test.jpg')
#tt = c_char_p*1
#res = (c_char_p)("gggg")
#res = create_string_buffer("ggggggg")
#g = libtest.test("gg");
# print ctyeps.sting_at(g)
# res = ""
#r = fdfs.MyTest("ls -l", res)
# o = fdfs.fdfs_init("/etc/fdfs/client.conf", 7)

#obj_handle = o

r = fdfs.init('/etc/fdfs/client.conf', log_level=7, log_fd=sys.stderr.fileno())

if r != 0:
    exit()

print('Upload file:')

with open('test.jpg', 'rb') as f:
    file_buff = f.read()

start = time.perf_counter()

r = fdfs.upload_file(file_buff, 'jpg')

elapsed = time.perf_counter() - start

print('Response:', r)
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print()
print('Delete file:')

group_name = r[1].split('/')[0]
remote_filename = r[1][r[1].find('/') + 1:]

start = time.perf_counter()

r = fdfs.delete_file(group_name, remote_filename)

elapsed = time.perf_counter() - start

print('Response:', r)
print('Time Elapsed %f ms.' % (elapsed * 1000))

fdfs.destroy()
