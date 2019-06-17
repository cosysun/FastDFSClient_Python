#ifndef FDFSCLIENT_H
#define FDFSCLIENT_H

#include <string>
#include "fastcommon/base64.h"
#include "fastcommon/logger.h"
#include "fastcommon/sockopt.h"
#include "fastdfs/fdfs_global.h"
#include "fastdfs/fdfs_client.h"

class CFDFSClient {
private:
    BufferInfo m_RecvBufferInfo;
    char *m_pRemoteFileName;

public:
    CFDFSClient(void);
    ~CFDFSClient(void);

    // 初始化客户端
    int init(const char *sFDFSConfig,
            int nLogLevel, int nLogFD, bool bLogTakeOverStd);

    // 下载文件
    int fdfs_dowloadfile(BufferInfo *pBuff,
            const char *group_name, const char *remote_filename);

    // 上传
    int fdfs_uploadfile(const char *file_content,
            const char *file_ext_name, int file_size, int &name_size,
            char *&remote_file_name);

    // slave上传
    int fdfs_slave_uploadfile(const char *file_content,
            const char *master_filename, const char *prefix_name,
            const char *file_ext_name, int file_size, int &name_size,
            char *&remote_file_name);

    // 删除
    int fdfs_deletefile(const char *group_name, const char *remote_filename);

    // 所有组信息
    int list_all_groups(BufferInfo *group_info);

    // 指定组信息
    int list_one_group(const char *group_name, BufferInfo *group_info);

    // storage信息
    int list_storages(const char *group_name,
            const char *storage_id,
            BufferInfo *storages_info);
};

#endif // FDFSCLIENT_H
