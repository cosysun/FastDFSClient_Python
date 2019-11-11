import FDFSPythonClient as fdfs
import json
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

dump = []
for i in json.loads(r[1]):
    dump.append(json.loads(i))
print(json.dumps(dump, indent=4))

print("##########################################")
print('list_one_group:')

start = time.perf_counter()
r = fdfs.list_one_group('group1')
elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

print(json.dumps(json.loads(r[1]), indent=4))

print("##########################################")
print('list_servers:')

start = time.perf_counter()
r = fdfs.list_servers('group1', '')
elapsed = time.perf_counter() - start

print('Response Code:', r[0])
print('Time Elapsed %f ms.' % (elapsed * 1000))

if r[0] != 0:
    exit()

dump = []
for i in json.loads(r[1]):
    dump.append(json.loads(i))
print(json.dumps(dump, indent=4))

print("##########################################")

fdfs.destroy()
