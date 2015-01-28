import sys
import os
sys.path.append(os.getcwd())

import FDFSPythonClient
import time

o = FDFSPythonClient.fdfs_init("/etc/fdfs/client.conf", 7)
lastTime = time.time()

for i in range(1):
    r = FDFSPythonClient.fdfs_download("group2", "M00/00/00/CgEIzVRlv3SAYJCTAAAsFwWtoVg9432330")
print (time.time() - lastTime)*1000

print r[0]
if r[0] == 0:
    file = open('2.png', 'wb')
    file.write(r[1])
    file.close()

