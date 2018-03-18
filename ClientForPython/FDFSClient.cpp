#include "FDFSClient.h"
#include "FDFSPythonClient.h"
#include "json/json.h"

#define  MAX_REMOTE_FILE_NAME_SIZE 100

CFDFSClient::CFDFSClient(void)
{
	m_pTrackerServer = NULL;
	//m_RecvBufferInfo = {0};
	memset(&m_RecvBufferInfo,0,sizeof(m_RecvBufferInfo));
	m_pRemoteFileName = NULL;
}

CFDFSClient::~CFDFSClient(void)
{
	fdfs_client_destroy();

	log_destroy();
	
	if (m_RecvBufferInfo.buff != NULL)
	{
		free(m_RecvBufferInfo.buff);
	}

	if (m_pRemoteFileName != NULL)
	{
		free(m_pRemoteFileName);
	}
}

int CFDFSClient::init( const char* sFDFSConfig, int nLogLevel)
{
	// 初始化日志
    log_init(); 
    g_log_context.log_level = nLogLevel;

	m_strConfigPath = sFDFSConfig;
	m_nLevelLog = nLogLevel;

	// 初始化fastfds客户端
	int result = 0;
	result = fastfdfs_client_init(sFDFSConfig);

    m_pRemoteFileName = (char*)malloc(MAX_REMOTE_FILE_NAME_SIZE * sizeof(char));
	memset(m_pRemoteFileName, 0, MAX_REMOTE_FILE_NAME_SIZE - 1);
	return result;
}

int CFDFSClient::fastfdfs_client_init(const char* sFDFSConfig)
{
	int result = 0;
	if ((result=fdfs_client_init(m_strConfigPath.c_str())) != 0)
	{
		logErrorEx(&g_log_context, "CFDFSClient::init() fdfs_client_init is failed, result:%d", result);
		return FSC_ERROR_CODE_INIT_FAILED;
	}

	return result;
}

int CFDFSClient::fdfs_dowloadfile( BufferInfo* pBuff, const char *group_name, const char* remote_filename)
{
	char *file_buff = NULL;
    int64_t file_size = 0;
    int result = 0;
    
    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::init() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}

    // 查询storage 服务地址
	ConnectionInfo storageServer;
	ConnectionInfo* pStorageServer;
    result = tracker_query_storage_fetch(pTrackerServer, \
		&storageServer, group_name, remote_filename);
    if (result != 0)
	{
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_dowloadfile() tracker_query_storage_fetch fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
            
        tracker_disconnect_server_ex(pTrackerServer, true);

		return FSC_ERROR_CODE_QUERY_STORAGE_FAIL;
	}

	logDebugEx(&g_log_context, "CFDFSClient::fdfs_dowloadfile() storage=%s:%d\n", storageServer.ip_addr, \
		storageServer.port);

	if ((pStorageServer=tracker_connect_server(&storageServer, \
		&result)) == NULL)
	{
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_dowloadfile() \
			tracker_connect_server failed, result:%d, storage=%s:%d\n", 
			result, storageServer.ip_addr, \
			storageServer.port);
        
        tracker_disconnect_server_ex(pTrackerServer, true);

		return FSC_ERROR_CODE_CONNECT_STORAGE_FAIL;
    }

    if ((result=storage_download_file_to_buff( \
		pTrackerServer, pStorageServer, \
		group_name, remote_filename, \
		&file_buff, &file_size)) == 0)
	{
        buffer_memcpy(&m_RecvBufferInfo, file_buff, file_size);
		*pBuff = m_RecvBufferInfo;
		free(file_buff);
	}

    if (result != 0)
	{
		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_dowloadfile() download file fail, " \
					"group:%s, remote;%s, error no: %d, error info: %s\n", \
					group_name, remote_filename, result, strMsg);
		result = FSC_ERROR_CODE_DOWNLAOD_FILE_FAIL;
	}

    tracker_disconnect_server_ex(pStorageServer, true);
    tracker_disconnect_server_ex(pTrackerServer, true);
    
    return  result;
}

int CFDFSClient::fdfs_uploadfile(const char *file_content, const char *file_ext_name, int file_size,
								 int& name_size, char*& remote_file_name)
{
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_uploadfile() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}
    
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	char remote_filename[256];

	int store_path_index;
	ConnectionInfo storageServer;
	ConnectionInfo* pStorageServer;
	if ((result=tracker_query_storage_store(pTrackerServer, \
		&storageServer, group_name, &store_path_index)) != 0)
	{
        tracker_disconnect_server_ex(pTrackerServer, true);

		logErrorEx(&g_log_context, "tracker_query_storage fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
		return result;
	}

	if ((pStorageServer=tracker_connect_server(&storageServer, \
		&result)) == NULL)
	{
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_uploadfile() \
								   tracker_connect_server failed, result:%d, storage=%s:%d\n", 
								   result, storageServer.ip_addr, \
								   storageServer.port);
        tracker_disconnect_server_ex(pTrackerServer, true);

		return result;
	}

	result = storage_upload_by_filebuff(pTrackerServer, \
		pStorageServer, store_path_index, \
		file_content, file_size, file_ext_name, \
		NULL, 0, \
		group_name, remote_filename);
	if (result != 0)
	{
		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_uploadfile() upload file fail, " \
			"group:%s, remote;%s, error no: %d, error info: %s\n", \
			group_name, remote_filename, result, strMsg);
	}
    else{
        int nNameSize = snprintf(m_pRemoteFileName,MAX_REMOTE_FILE_NAME_SIZE-1, "%s/%s", group_name, remote_filename);
        remote_file_name = m_pRemoteFileName;
        name_size = nNameSize;
    }
    
    tracker_disconnect_server_ex(pStorageServer, true);
    tracker_disconnect_server_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::fdfs_slave_uploadfile( const char *file_content, const char *master_filename,
                                        const char *prefix_name, const char *file_ext_name, 
                                        int file_size, int& name_size, char*& remote_file_name)
{
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_slave_uploadfile() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}
    
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	char remote_filename[256];

	int store_path_index;
	ConnectionInfo storageServer;
	ConnectionInfo* pStorageServer;
	if ((result=tracker_query_storage_store(pTrackerServer, \
		&storageServer, group_name, &store_path_index)) != 0)
	{
        tracker_disconnect_server_ex(pTrackerServer, true);

		logErrorEx(&g_log_context, "tracker_query_storage fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
		return result;
	}

	if ((pStorageServer=tracker_connect_server(&storageServer, \
		&result)) == NULL)
	{
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_slave_uploadfile() \
								   tracker_connect_server failed, result:%d, storage=%s:%d\n", 
								   result, storageServer.ip_addr, \
								   storageServer.port);
        tracker_disconnect_server_ex(pTrackerServer, true);

		return result;
	}

	result = storage_upload_slave_by_filebuff(pTrackerServer, \
		pStorageServer, file_content, file_size, \
        master_filename, prefix_name, file_ext_name, \
		NULL, 0, \
		group_name, remote_filename);
	
	//logErrorEx(&g_log_context, "master_filename:%s, prefix_name:%s, file_ext_name:%s, group_name:%s, remote_filename:%s\n", 
	//	master_filename, prefix_name, file_ext_name, group_name, remote_filename);
	
	if (result != 0)
	{
		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_slave_uploadfile() upload file fail, " \
			"group:%s, remote;%s, error no: %d, error info: %s\n", \
			group_name, remote_filename, result, strMsg);
	}
    else{
        int nNameSize = snprintf(m_pRemoteFileName,MAX_REMOTE_FILE_NAME_SIZE-1, "%s/%s", group_name, remote_filename);
        remote_file_name = m_pRemoteFileName;
        name_size = nNameSize;
    }
    
    tracker_disconnect_server_ex(pStorageServer, true);
    tracker_disconnect_server_ex(pTrackerServer, true);

    return result;
}


int CFDFSClient::fdfs_deletefile( const char *group_name, const char* remote_filename )
{
	int result = 0;

    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::init() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}
    
	// 查询storage 服务地址
	ConnectionInfo storageServer;
	ConnectionInfo* pStorageServer;
	result = tracker_query_storage_update(pTrackerServer, \
		&storageServer, group_name, remote_filename);
	if (result != 0)
	{
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_deletefile() tracker_query_storage_fetch fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));  
        
        tracker_disconnect_server_ex(pTrackerServer, true);

		return FSC_ERROR_CODE_QUERY_STORAGE_FAIL;
	}

	logDebugEx(&g_log_context, "CFDFSClient::fdfs_deletefile() storage=%s:%d\n", storageServer.ip_addr, \
		storageServer.port);

	if ((pStorageServer=tracker_connect_server(&storageServer, \
		&result)) == NULL)
	{
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_deletefile() \
								   tracker_connect_server failed, result:%d, storage=%s:%d\n", 
								   result, storageServer.ip_addr, \
								   storageServer.port);
        tracker_disconnect_server_ex(pTrackerServer, true);

		return FSC_ERROR_CODE_CONNECT_STORAGE_FAIL;
	}

	// 删除操作
	result=storage_delete_file(pTrackerServer, NULL, group_name, remote_filename);
	if (result != 0)
	{
		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::fdfs_deletefile() delete file fail, " \
			"group:%s, remote;%s, error no: %d, error info: %s\n", \
			group_name, remote_filename, result, strMsg);

		result = FSC_ERROR_CODE_DELETE_FILE_FAIL;
	}

	tracker_disconnect_server_ex(pStorageServer, true);
    tracker_disconnect_server_ex(pTrackerServer, true);
    
	return  result;
}

int CFDFSClient::list_all_groups(BufferInfo* group_info)
{
	int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::init() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}

	int group_count;
	FDFSGroupStat group_stats[FDFS_MAX_GROUPS];
	result = tracker_list_groups(pTrackerServer, \
		group_stats, FDFS_MAX_GROUPS, \
		&group_count);
	if (result != 0)
	{
        tracker_disconnect_server_ex(pTrackerServer, true);

		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::list_all_groups() get  list_all_groups fail, " \
			"error no: %d, error info: %s\n", \
			result, strMsg);

		return result;
	}

	Json::Value GroupsValue;
	Json::Value Info;
	FDFSGroupStat *pGroupStat;
	FDFSGroupStat *pGroupEnd;
	pGroupEnd = group_stats + group_count;
	for (pGroupStat=group_stats; pGroupStat<pGroupEnd; \
	 pGroupStat++)
	{
		Info["group_name"] = pGroupStat->group_name;
		Info["total_mb"] = (long long)pGroupStat->total_mb;
		Info["free_mb"] = (long long)pGroupStat->free_mb;
		Info["trunk_free_mb"] = (long long)pGroupStat->trunk_free_mb;
		Info["server_count"] = pGroupStat->count;
		Info["storage_port"] = pGroupStat->storage_port;
		Info["active_count"] = pGroupStat->active_count;
		Info["current_write_server"] = pGroupStat->current_write_server;
		Info["subdir_count_per_path"] = pGroupStat->subdir_count_per_path;
		Info["current_trunk_file_id"] = pGroupStat->current_trunk_file_id;

		GroupsValue.append(Info);
	}
	std::string strGroupsInfo = GroupsValue.toStyledString();
	buffer_memcpy(&m_RecvBufferInfo, strGroupsInfo.c_str(), strGroupsInfo.length());
    *group_info = m_RecvBufferInfo;
   
    tracker_disconnect_server_ex(pTrackerServer, true);

	return result;
}

int CFDFSClient::list_one_group(const char* group_name, BufferInfo* group_info)
{
	int result;
	FDFSGroupStat group_stat;
    
    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::init() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}

	result = tracker_list_one_group(pTrackerServer, group_name, &group_stat);
	if (result != 0)
	{
        tracker_disconnect_server_ex(pTrackerServer, true);

		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::list_one_group() get list_one_group fail, " \
			"error no: %d, error info: %s\n", \
			result, strMsg);

		return result;
	}

	Json::Value GroupsValue;
	GroupsValue["group_name"] = group_stat.group_name;
	GroupsValue["total_mb"] = (long long)group_stat.total_mb;
	GroupsValue["free_mb"] = (long long)group_stat.free_mb;
	GroupsValue["trunk_free_mb"] = (long long)group_stat.trunk_free_mb;
	GroupsValue["server_count"] = group_stat.count;
	GroupsValue["storage_port"] = group_stat.storage_port;
	GroupsValue["active_count"] = group_stat.active_count;
	GroupsValue["current_write_server"] = group_stat.current_write_server;
	GroupsValue["subdir_count_per_path"] = group_stat.subdir_count_per_path;
	GroupsValue["current_trunk_file_id"] = group_stat.current_trunk_file_id;

	std::string strGroupsInfo = GroupsValue.toStyledString();
	buffer_memcpy(&m_RecvBufferInfo, strGroupsInfo.c_str(), strGroupsInfo.length());
	*group_info = m_RecvBufferInfo;
        
    tracker_disconnect_server_ex(pTrackerServer, true);

	return result;
}

int CFDFSClient::list_storages(const char* group_name, 
							   const char* storage_id, 
							   BufferInfo* storages_info)
{
	int result;
	int storage_count;
	FDFSStorageInfo storage_infos[FDFS_MAX_SERVERS_EACH_GROUP];
	FDFSStorageInfo *pStorage;
	FDFSStorageInfo *pStorageEnd;
	FDFSStorageStat *pStorageStat;

    ConnectionInfo *pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		result = (errno != 0 ? errno : ECONNREFUSED);
		logErrorEx(&g_log_context, "CFDFSClient::init() tracker_get_connection is failed, result:%d", result);

		return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
	}
    
	result = tracker_list_servers(pTrackerServer, \
		group_name, storage_id, \
		storage_infos, FDFS_MAX_SERVERS_EACH_GROUP, \
		&storage_count);
	if (result != 0)
	{
        tracker_disconnect_server_ex(pTrackerServer, true);

		const char* strMsg = STRERROR(result);
		logErrorEx(&g_log_context, "CFDFSClient::list_storages() get list_storages fail, " \
			"error no: %d, error info: %s\n", \
			result, strMsg);

		return result;
	}

	Json::Value StoragesValue;
	Json::Value Info;
	char szJoinTime[32];
	char szUpTime[32];
	char szLastHeartBeatTime[32];
	char szSrcUpdTime[32];
	char szSyncUpdTime[32];
	char szSyncedTimestamp[32];
	pStorageEnd = storage_infos + storage_count;
	for (pStorage=storage_infos; pStorage<pStorageEnd; \
		pStorage++)
	{
		Info["status"] = pStorage->status;
		Info["id"] = pStorage->id;
		Info["ip_addr"] = pStorage->ip_addr;
		Info["version"] = pStorage->version;
		Info["src_id"] = pStorage->src_id;
		Info["domain_name"] = pStorage->domain_name;
		Info["total_mb"] = pStorage->total_mb;
		Info["free_mb"] = pStorage->free_mb;
		Info["upload_priority"] = pStorage->upload_priority;

		formatDatetime(pStorage->join_time, \
			"%Y-%m-%d %H:%M:%S", \
			szJoinTime, sizeof(szJoinTime)), \
		Info["join_time"] = szJoinTime;

		formatDatetime(pStorage->up_time, \
		"%Y-%m-%d %H:%M:%S", \
		szUpTime, sizeof(szUpTime)), \
		Info["up_time"] = szUpTime;

		Info["store_path_count"] = pStorage->store_path_count;	
		Info["subdir_count_per_path"] = pStorage->subdir_count_per_path;	
		Info["storage_port"] = pStorage->storage_port;	
		Info["current_write_path"] = pStorage->current_write_path;	

		// 统计数据
		pStorageStat = &(pStorage->stat);
		Info["total_upload_count"] = (long long)pStorageStat->total_upload_count;	
		Info["success_upload_count"] = (long long)pStorageStat->success_upload_count;	
		Info["total_delete_count"] = (long long)pStorageStat->total_delete_count;	
		Info["success_delete_count"] = (long long)pStorageStat->success_delete_count;	
		Info["total_download_count"] = (long long)pStorageStat->total_download_count;	
		Info["success_download_count"] = (long long)pStorageStat->success_download_count;	

		Info["total_upload_bytes"] = (long long)pStorageStat->total_upload_bytes;	
		Info["success_upload_bytes"] = (long long)pStorageStat->success_upload_bytes;
		Info["total_download_bytes"] = (long long)pStorageStat->total_download_bytes;	
		Info["success_download_bytes"] = (long long)pStorageStat->success_download_bytes;
		Info["total_sync_in_bytes"] = (long long)pStorageStat->total_sync_in_bytes;	
		Info["success_sync_in_bytes"] = (long long)pStorageStat->success_sync_in_bytes;
		Info["total_sync_out_bytes"] = (long long)pStorageStat->total_sync_out_bytes;	
		Info["success_sync_out_bytes"] = (long long)pStorageStat->success_sync_out_bytes;

		Info["total_file_open_count"] = (long long)pStorageStat->total_file_open_count;	
		Info["success_file_open_count"] = (long long)pStorageStat->success_file_open_count;
		Info["total_file_read_count"] = (long long)pStorageStat->total_file_read_count;	
		Info["success_file_read_count"] = (long long)pStorageStat->success_file_read_count;
		Info["total_file_write_count"] = (long long)pStorageStat->total_file_write_count;	
		Info["success_file_write_count"] =(long long) pStorageStat->success_file_write_count;

		formatDatetime(pStorageStat->last_heart_beat_time, \
			"%Y-%m-%d %H:%M:%S", \
			szLastHeartBeatTime, sizeof(szLastHeartBeatTime)), \
		Info["last_heart_beat_time"] = szLastHeartBeatTime;

		formatDatetime(pStorageStat->last_source_update, \
		"%Y-%m-%d %H:%M:%S", \
		szSrcUpdTime, sizeof(szSrcUpdTime)), \
		Info["last_source_update"] = szSrcUpdTime;

		formatDatetime(pStorageStat->last_sync_update, \
		"%Y-%m-%d %H:%M:%S", \
		szSyncUpdTime, sizeof(szSyncUpdTime)), \
		Info["last_sync_update"] = szSyncUpdTime;

		formatDatetime(pStorageStat->last_synced_timestamp, \
		"%Y-%m-%d %H:%M:%S", \
		szSyncedTimestamp, sizeof(szSyncedTimestamp));
		Info["last_synced_timestamp"] = szSyncedTimestamp;

		StoragesValue.append(Info);
	}

	std::string strStorageInfo = StoragesValue.toStyledString();
	buffer_memcpy(&m_RecvBufferInfo, strStorageInfo.c_str(), strStorageInfo.length());
	*storages_info = m_RecvBufferInfo;
    
    tracker_disconnect_server_ex(pTrackerServer, true);

	return result;
}
