#ifndef MY_FDFS_CLIENT_H
#define MY_FDFS_CLIENT_H

#include "fdfs_client.h"
#include "fdfs_global.h"
#include "base64.h"
#include "sockopt.h" 
#include "logger.h"
#include <string>

using namespace std;

class CFDFSClient
{
public:
	CFDFSClient(void);
	~CFDFSClient(void);

public:
	
	// 初始化客户端
	int init(const char* sFDFSConfig, int nLogLevel);

	// 下载文件
	int fdfs_dowloadfile(BufferInfo* pBuff, const char *group_name, const char* remote_filename);
	
	// 上传
	int fdfs_uploadfile( const char *file_content, const char *file_ext_name, int file_size, 
		int& name_size, char*& remote_file_name);
    
    // slave上传
    int fdfs_slave_uploadfile( const char *file_content, const char *master_filename,
                               const char *prefix_name, const char *file_ext_name, 
                               int file_size, int& name_size, char*& remote_file_name );

	// 删除
	int fdfs_deletefile(const char *group_name, const char* remote_filename);

	// 所有组信息
	int list_all_groups(BufferInfo* group_info);

	// 指定组信息
	int list_one_group(const char* group_name, BufferInfo *group_info);

	// storage信息
	int list_storages(const char* group_name, 
		const char* storage_id, 
		BufferInfo* storages_info);

private:
	void re_fastfds_client_init();

	int fastfdfs_client_init(const char* sFDFSConfig);

private:
	ConnectionInfo *m_pTrackerServer;
	BufferInfo m_RecvBufferInfo;
	char* m_pRemoteFileName;
	string m_strConfigPath;
	int	m_nLevelLog;
};

#endif

