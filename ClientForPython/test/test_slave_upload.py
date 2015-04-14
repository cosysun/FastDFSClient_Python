import sys
import os
import FDFSPythonClient 
import time

sys.path.append(os.getcwd())
o = FDFSPythonClient.fdfs_init("/etc/fdfs/client.conf", 7)

# read pic content
file_obj = open('1.png', 'rb')

try:
    file_content = file_obj.read()
finally:
    file_obj.close()

lastTime = time.time()

# send normal picture 
r = FDFSPythonClient.fdfs_upload(file_content, "")
prefix_name = "200x200"
print r

# send slave pic
master_filename = r[1][r[1].find('/')+1:]
slave_r = FDFSPythonClient.fdfs_slave_upload(file_content, "png",master_filename, prefix_name)

print (time.time() - lastTime)*1000

print slave_r

