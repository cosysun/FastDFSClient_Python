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

r = fdfs.upload_file(file_buff, 'jpg')

elapsed = time.perf_counter() - start

print('Response:', r)
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print()
print('Download file:')

group_name = r[1].split('/')[0]
remote_filename = r[1][r[1].find('/') + 1:]

start = time.perf_counter()

r = fdfs.download_file(group_name, remote_filename)

elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

fdfs.destory()

if r[0] == 0:
    with open(remote_filename.rsplit('/')[-1], 'wb') as f:
        f.write(r[1])
