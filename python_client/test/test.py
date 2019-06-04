import time
import FDFSPythonClient


o = FDFSPythonClient.fdfs_init("/etc/fdfs/client.conf", 7)

startTime = time.time()
r = FDFSPythonClient.fdfs_download(
    "group1", "M00/00/00/fwAAAV0AsByAK2cuAAK1gKDElgs203.jpg")
print((time.time() - startTime)*1000)

print(r[0])
if r[0] == 0:
    with open('2.png', 'wb') as f:
        f.write(r[1])
