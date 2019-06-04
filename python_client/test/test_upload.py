import sys
import os
import FDFSPythonClient
import time

sys.path.append(os.getcwd())
o = FDFSPythonClient.fdfs_init("/etc/fdfs/client.conf", 7)

file_obj = open('1.png', 'rb')

try:
    file_content = file_obj.read()
finally:
    file_obj.close()

lastTime = time.time()

for i in range(1):
    r = FDFSPythonClient.fdfs_upload(file_content, "")

print((time.time() - lastTime)*1000)

print(r)
