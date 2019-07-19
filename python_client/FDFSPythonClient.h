#ifndef FDFSPYTHONCLIENT_H
#define FDFSPYTHONCLIENT_H

#include "fastdfs/fdfs_global.h"

//错误码
enum FSC_CODE {
    FSC_ERROR_CODE_NONE = 0, //没有错误
    FSC_ERROR_CODE_INIT_FAILED, //初始化失败

    FSC_ERROR_CODE_PARAM_INVAILD, //参数无效

    FSC_ERROR_CODE_CONNECT_TRACKER_FAIL, //连接失败
    FSC_ERROR_CODE_QUERY_STORAGE_FAIL, //查询storage地址失败
    FSC_ERROR_CODE_CONNECT_STORAGE_FAIL, //连接storage失败

    FSC_ERROR_CODE_DOWNLAOD_FILE_FAIL, //下载文件失败
    FSC_ERROR_CODE_DELETE_FILE_FAIL, //删除文件失败
};

#ifdef __cplusplus
extern "C" {
#endif
    //////////////////////////////////////////////////////////////////////////
    //功能：初始化fastdfs
    //参数：
    //      const char* sConfig IN FastDFS配置文件路劲 比如:/etc/fdfs/client.conf
    //      int nLogLevel IN （可选）日志等级 采用的是unix 日志等级
    //          0: LOG_EMERG
    //          1: LOG_ALERT
    //          2: LOG_CRIT
    //          3: LOG_ERR
    //          4: LOG_WARNING
    //          5: LOG_NOTICE
    //          6: LOG_INFO
    //          7: LOG_DEBUG
    //      int nLogFD IN （可选）重定向输出日志的文件描述符
    //          默认日志路径为配置文件中base_path/logs/cliented.log
    //      bool bLogTakeOverStd IN （可选）是否将stdout与stderr重定向输出到文件
    //          默认为False
    //返回：int& anError OUT 错误信息
    //备注：
    //      注意初始化时，必须保证conf文件中base_path目录存在
    //      比如 base_path=/fastdfs/tracker, 需要保证/fastdfs/tracker存在，
    //      不存在 需创建mkdir /fastdfs/tracker
    //      *请保证程序有对base_path目录下的读写权限，否则会输出到/tmp/logs/cliented.log里
    //////////////////////////////////////////////////////////////////////////
    int init(const char *sConfig,
            int nLogLevel, int nLogFD, bool bLogTakeOverStd);

    // 销毁fdfs客户端
    int destory();

    //////////////////////////////////////////////////////////////////////////
    //功能：上传普通文件
    //参数：
    //      char *file_buff IN 文件内容
    //      const char *file_ext_name IN 文件扩展名
    //      int file_size IN 文件大小
    //      int& name_size  OUT 返回的文件名大小
    //      char* remote_file_name OUT 返回的文件名
    //      比如：group2/M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //返回：0:成功 否则失败。
    //////////////////////////////////////////////////////////////////////////
    int upload_file(const char *file_buff, const char *file_ext_name,
            int file_size, int &name_size, char *&remote_file_name);

    //////////////////////////////////////////////////////////////////////////
    //功能：上传追加类型的文件
    //参数：
    //      char *file_buff IN 文件内容
    //      const char *file_ext_name IN 文件扩展名
    //      int file_size IN 文件大小
    //      int& name_size  OUT 返回的文件名大小
    //      char* remote_file_name OUT 返回的文件名
    //      比如：group2/M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //返回：0:成功 否则失败。
    //////////////////////////////////////////////////////////////////////////
    int upload_appender(const char *file_buff, const char *file_ext_name,
            int file_size, int &name_size, char *&remote_file_name);

    //////////////////////////////////////////////////////////////////////////
    //功能：向追加类型的文件追加上传文件
    //参数：
    //      char *file_buff IN 文件内容
    //      int file_size IN 文件大小
    //      const char* appender_filename IN 要追加上传的追加类型的文件名
    //      比如：M00/00/00/fwAAAV0VrVWEW6E8AAAAAKDElgs642.ext
    //返回：0:成功 否则失败。
    //////////////////////////////////////////////////////////////////////////
    int append_file(const char *file_buff, int file_size,
            const char *appender_filename);

    //////////////////////////////////////////////////////////////////////////
    //功能：slave文件上传
    //参数：
    //      char *file_content IN 文件内容
    //      const char *master_filename IN 主文件名 存在服务端文件名
    //      比如：M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250
    //      const char *prefix_name IN 从前缀名 比如200*200
    //      const char *file_ext_name IN 文件扩展名
    //      int file_size IN 文件大小
    //      int& name_size  OUT 返回的文件名大小
    //      char* remote_file_name OUT 返回的文件名
    //      比如：group2/M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //返回：0:成功 否则失败。
    //////////////////////////////////////////////////////////////////////////
    int upload_slave(const char *file_content, const char *master_filename,
            const char *prefix_name, const char *file_ext_name, int file_size,
            int &name_size, char *&remote_file_name);

    //////////////////////////////////////////////////////////////////////////
    //功能：下载文件
    //参数：
    //      BufferInfo* pBuff OUT 返回的文件信息，文件大小，以及内存大小
    //      const char *group_name IN 存储组名
    //      const char* remote_filename IN 存储名
    //      比如：group2/M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //      group_name:group2
    //      remote_filename: M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //返回：0:成功 否则失败。
    //////////////////////////////////////////////////////////////////////////
    int download_file(BufferInfo *pBuff,
            const char *group_name, const char *remote_filename);

    //////////////////////////////////////////////////////////////////////////
    //功能：删除文件
    //参数：
    //      const char *group_name IN 存储组名
    //      const char* remote_filename IN 存储名
    //      比如：group2/M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //      group_name:group2
    //      remote_filename: M00/00/00/CgEIzVRhnJWAZfVkAAAsFwWtoVg250.png
    //返回：0:成功 否则失败。
    //////////////////////////////////////////////////////////////////////////
    int delete_file(const char *group_name, const char *remote_filename);

    //////////////////////////////////////////////////////////////////////////
    //功能：list all groups
    //参数：
    //      BufferInfo* group_info OUT 所有组信息
    //返回：0:成功 否则失败。
    //备注：需要在主版本更新时检查信息是否需要再次更新，当前匹配版本V5.12
    //////////////////////////////////////////////////////////////////////////
    int list_groups(BufferInfo *group_info);

    //////////////////////////////////////////////////////////////////////////
    //功能：list one group
    //参数：
    //      const char* group_name IN 组名 如：group1
    //      BufferInfo* group_info OUT 所有组信息
    //返回：0:成功 否则失败。
    //备注：需要在主版本更新时检查信息是否需要再次更新，当前匹配版本V5.12
    //////////////////////////////////////////////////////////////////////////
    int list_one_group(const char *group_name, BufferInfo *group_info);

    //////////////////////////////////////////////////////////////////////////
    //功能：list all servers of the specified group
    //参数：
    //      const char* group_name IN 组名 如：group1
    //      const char* storage_id IN 组名 如：storage ip
    //      BufferInfo* storages_info OUT 存储信息
    //返回：0:成功 否则失败。
    //备注：需要在主版本更新时检查信息是否需要再次更新，当前匹配版本V5.12
    //////////////////////////////////////////////////////////////////////////
    int list_servers(const char *group_name, const char *storage_id,
            BufferInfo *storages_infos);
#ifdef __cplusplus
}
#endif

#endif // FDFSPYTHONCLIENT_H
