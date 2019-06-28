#ifndef FDFSCLIENT_H
#define FDFSCLIENT_H

#include "fastcommon/base64.h"
#include "fastcommon/logger.h"
#include "fastcommon/sockopt.h"
#include "fastdfs/fdfs_global.h"
#include "fastdfs/fdfs_client.h"

class CFDFSClient {
public:
    CFDFSClient(void);
    ~CFDFSClient(void);

    // 初始化客户端
    int init(const char *sFDFSConfig,
            int nLogLevel, int nLogFD, bool bLogTakeOverStd);

    // 上传普通文件
    int upload_file(const char *file_buff,
            const char *file_ext_name, int file_size, int &name_size,
            char *&remote_file_name);

    // 上传追加类型文件
    int upload_appender(const char *file_buff,
            const char *file_ext_name, int file_size, int &name_size,
            char *&remote_file_name);

    // 向追加类型文件追加上传
    int append_file(const char *file_buff, int file_size,
            const char *appender_filename);

    // slave文件上传
    int upload_slave(const char *file_buff,
            const char *master_filename, const char *prefix_name,
            const char *file_ext_name, int file_size, int &name_size,
            char *&remote_file_name);

    // 下载文件
    int download_file(BufferInfo *pBuff,
            const char *group_name, const char *remote_filename);

    // 删除
    int delete_file(const char *group_name, const char *remote_filename);

    // 所有组信息
    int list_all_groups(BufferInfo *group_info);

    // 指定组信息
    int list_one_group(const char *group_name, BufferInfo *group_info);

    // storage信息
    int list_storages(const char *group_name,
            const char *storage_id,
            BufferInfo *storages_info);

private:
    BufferInfo m_RecvBufferInfo;
    char *m_pRemoteFileName;

};

#endif // FDFSCLIENT_H
