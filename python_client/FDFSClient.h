#ifndef FDFSCLIENT_H
#define FDFSCLIENT_H

#include "fastdfs/fdfs_global.h"

class CFDFSClient {
public:
    CFDFSClient(void);
    ~CFDFSClient(void);

    // 初始化客户端
    int init(const char *sFDFSConfig,
            int nLogLevel, int nLogFD, bool bLogTakeOverStd);

    // 上传普通文件
    int upload_file(const char *file_buff, int64_t file_size, const char *file_ext_name,
            char *&remote_file_name, int &name_size);

    // slave文件上传
    int upload_slave(const char *file_buff, int64_t file_size,
            const char *master_file_id, const char *prefix_name, const char *file_ext_name,
            char *&remote_file_id, int &name_size);

    // 上传追加类型文件
    int upload_appender(const char *file_buff, int64_t file_size, const char *file_ext_name,
            char *&remote_file_name, int &name_size);

    // 向追加类型文件追加上传
    int append_file(const char *file_buff, const int64_t file_size,
            const char *appender_file_id);

    // 下载文件
    int download_file(const char *group_name, const char *remote_filename,
            BufferInfo *pBuff);

    // 删除
    int delete_file(const char *group_name, const char *remote_filename);

    // 获取文件信息
    int get_file_info(const char *file_id, BufferInfo *file_info);

    // list all groups
    int list_groups(BufferInfo *groups_info);

    // list one group
    int list_one_group(const char *group_name, BufferInfo *group_info);

    // list all servers of the specified group
    int list_servers(const char *group_name, const char *storage_id,
            BufferInfo *storages_info);

private:
    BufferInfo m_RecvBufferInfo;
    char *m_pRemoteFileName;

};

#endif // FDFSCLIENT_H
