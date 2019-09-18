import FDFSPythonClient as fdfs
import time
import sys

r = fdfs.init('/etc/fdfs/client.conf', log_level=7, log_fd=sys.stderr.fileno())

if r != 0:
    exit()

print('Upload file:')

with open('test.jpg', 'rb') as f:
    file_buff = f.read()

start = time.perf_counter()

n = fdfs.upload_file(file_buff, 'jpg')

elapsed = time.perf_counter() - start

print('Response:', n)
print('Time Elapsed %f ms.' % (elapsed * 1000))

if n[0] != 0:
    exit()

print()
print('Get file info:')

start = time.perf_counter()

r = fdfs.get_file_info(n[1])

elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print(r[1])

fdfs.destroy()

