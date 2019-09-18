import FDFSPythonClient as fdfs
import time
import sys

r = fdfs.init('/etc/fdfs/client.conf', log_level=7, log_fd=sys.stderr.fileno())

if r != 0:
    exit()

print("##########################################")
print('list_groups:')

start = time.perf_counter()
r = fdfs.list_groups()
elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print(r[1])

print("##########################################")
print('list_one_group:')

start = time.perf_counter()
r = fdfs.list_one_group('group1')
elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print(r[1])

print("##########################################")
print('list_servers:')

start = time.perf_counter()
r = fdfs.list_servers('group1', '')
elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print(r[1])

print("##########################################")

fdfs.destroy()
