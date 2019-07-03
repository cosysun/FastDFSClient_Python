import FDFSPythonClient as fdfs
import time
import sys

r = fdfs.init('/etc/fdfs/client.conf', log_level=7, log_fd=sys.stderr.fileno())

if r != 0:
    exit()

print('Upload appender:')

with open('test.jpg', 'rb') as f:
    file_buff = f.read()

start = time.perf_counter()

r = fdfs.upload_appender(file_buff, 'jpg')

elapsed = time.perf_counter() - start

print('Response:', r)
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print()
print('Append file:')

appender_filename = r[1][r[1].find('/') + 1:]

start = time.perf_counter()

r = fdfs.append_file(file_buff, appender_filename)

elapsed = time.perf_counter() - start

print('Response:', r)
print('Time Elapsed %f ms.' % (elapsed * 1000))

fdfs.destory()
