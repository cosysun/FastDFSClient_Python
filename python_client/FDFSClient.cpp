#include "FDFSClient.h"

#include <sys/stat.h>   // fstat()

#include "fastcommon/json_parser.h"
#include "fastcommon/logger.h"
#include "fastdfs/fdfs_client.h"

#include "FDFSPythonClient.h"

#define BUFF_SIZE                 128
#define REMOTE_FILE_NAME_MAX_SIZE FDFS_GROUP_NAME_MAX_LEN + BUFF_SIZE
#define CLIENT_ERROR_LOG_FILENAME "cliented"

void init_json_array(json_array_t *array, int n) {
    array->buff = NULL;
    array->count = array->alloc = n;
    array->element_size = sizeof (string_t);
    array->elements = new string_t[array->count];
    memset(array->elements, 0, array->count * array->element_size);
}

void free_json_array(json_array_t *array, int n) {
    for (int i = 0; i < n; ++i) {
        free_json_string(&array->elements[i]);
    }

    delete[] array->elements;
    array->elements = NULL;
    array->count = 0;
}

void init_json_map(json_map_t *map, int n) {
    map->buff = NULL;
    map->count = map->alloc = n;
    map->element_size = sizeof (key_value_pair_t);
    map->elements = new key_value_pair_t[map->count];
    memset(map->elements, 0, map->count * map->element_size);
}

void free_json_map(json_map_t *map, int n) {
    for (int i = 0; i < n; ++i) {
        delete[] map->elements[i].key.str;
        delete[] map->elements[i].value.str;
    }

    delete[] map->elements;
    map->elements = NULL;
    map->count = 0;
}

template<typename T>
void assign_json_map(key_value_pair_t &kv_pair, const char *key,
        const char *format, T value) {
    delete[] kv_pair.key.str;
    kv_pair.key.str = new char[BUFF_SIZE];
    snprintf(kv_pair.key.str, BUFF_SIZE, "%s", key);
    kv_pair.key.len = strlen(kv_pair.key.str);

    delete[] kv_pair.value.str;
    kv_pair.value.str = new char[BUFF_SIZE];
    snprintf(kv_pair.value.str, BUFF_SIZE, format, value);
    kv_pair.value.len = strlen(kv_pair.value.str);
}

CFDFSClient::CFDFSClient(void) {
    memset(&m_RecvBufferInfo, 0, sizeof (m_RecvBufferInfo));
    m_pRemoteFileName = new char[REMOTE_FILE_NAME_MAX_SIZE];
    memset(m_pRemoteFileName, 0, REMOTE_FILE_NAME_MAX_SIZE * sizeof (char));
}

CFDFSClient::~CFDFSClient(void) {
    fdfs_client_destroy();
    log_destroy();

    delete m_RecvBufferInfo.buff;
    delete m_pRemoteFileName;
}

int CFDFSClient::init(const char *sFDFSConfig,
        int nLogLevel, int nLogFD, bool bLogTakeOverStd) {
    int result = 0;

    // 初始化日志
    if (bLogTakeOverStd) {
        if ((result = log_init2()) != 0) {
            fprintf(stderr, "CFDFSClient::init() log_init2 is failed, "
                    "error no: %d, error info: %s", result, STRERROR(result));
            return FSC_ERROR_CODE_INIT_FAILED;
        }
    } else {
        if ((result = log_init()) != 0) {
            fprintf(stderr, "CFDFSClient::init() log_init is failed, "
                    "error no: %d, error info: %s", result, STRERROR(result));
            return FSC_ERROR_CODE_INIT_FAILED;
        }
    }

    // 初始化fastfds客户端
    if ((result = fdfs_client_init(sFDFSConfig)) != 0) {
        fprintf(stderr, "CFDFSClient::init() fdfs_client_init is failed, "
                "error no: %d, error info: %s", result, STRERROR(result));
        return FSC_ERROR_CODE_INIT_FAILED;
    }

    if (nLogLevel >= LOG_EMERG && nLogLevel <= LOG_DEBUG) {
        g_log_context.log_level = nLogLevel;
    }

    if (nLogFD > 0) {
        struct stat buf;
        if (fstat(nLogFD, &buf) != 0) {
            fprintf(stderr, "Bad log_fd, errno: %d, error info: %s",
                    errno, STRERROR(errno));
            return FSC_ERROR_CODE_INIT_FAILED;
        }

        g_log_context.log_fd = nLogFD;
        return result;
    }

    if ((result = log_set_prefix(g_fdfs_base_path,
            CLIENT_ERROR_LOG_FILENAME)) != 0) {
        fprintf(stderr, "CFDFSClient::init() log_set_prefix is failed, "
                "error no: %d, error info: %s", result, STRERROR(result));
        return FSC_ERROR_CODE_INIT_FAILED;
    }

    return result;
}

int CFDFSClient::upload_file(const char *file_buff, int64_t file_size, const char *file_ext_name,
        char *&remote_file_name, int &name_size) {
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::upload_file() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
    char remote_filename[BUFF_SIZE];

    int store_path_index;
    ConnectionInfo storageServer;

    if ((result = tracker_query_storage_store(pTrackerServer, &storageServer,
            group_name, &store_path_index)) != 0) {
        tracker_close_connection_ex(pTrackerServer, true);

        logError("CFDFSClient::upload_file() tracker_query_storage_store fail,"
                " error no: %d, error info: %s", result, STRERROR(result));
        return result;
    }

    if (file_ext_name != NULL && (result = strlen(file_ext_name)) > FDFS_FILE_EXT_NAME_MAX_LEN) {
        logWarning("CFDFSClient::upload_file() : file ext name length "
                "should <= %d !, however got %d : %s",
                FDFS_FILE_EXT_NAME_MAX_LEN, result, file_ext_name);
    }

    result = storage_upload_by_filebuff(pTrackerServer, &storageServer,
            store_path_index, file_buff, file_size, file_ext_name,
            NULL, 0, group_name, remote_filename);
    if (result != 0) {
        logError("CFDFSClient::upload_file() storage_upload_by_filebuff fail,"
                " group:%s, remote:%s, error no: %d, error info: %s",
                group_name, remote_filename, result, STRERROR(result));
    } else {
        name_size = snprintf(m_pRemoteFileName, REMOTE_FILE_NAME_MAX_SIZE,
                "%s%c%s", group_name, FDFS_FILE_ID_SEPERATOR, remote_filename);
        remote_file_name = m_pRemoteFileName;
    }

    tracker_close_connection_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::upload_slave(const char *file_buff, int64_t file_size,
        const char *master_file_id, const char *prefix_name, const char *file_ext_name,
        char *&remote_file_id, int &name_size) {
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::upload_slave() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    char file_id[REMOTE_FILE_NAME_MAX_SIZE];

    if (prefix_name != NULL && (result = strlen(prefix_name)) > FDFS_FILE_PREFIX_MAX_LEN) {
        logWarning("CFDFSClient::upload_slave() : prefix name length "
                "should <= %d !, however got %d : %s",
                FDFS_FILE_PREFIX_MAX_LEN, result, prefix_name);
    }
    if (file_ext_name != NULL && (result = strlen(file_ext_name)) > FDFS_FILE_EXT_NAME_MAX_LEN) {
        logWarning("CFDFSClient::upload_slave() : file ext name length "
                "should <= %d !, however got %d : %s",
                FDFS_FILE_EXT_NAME_MAX_LEN, result, file_ext_name);
    }

    result = storage_upload_slave_by_filebuff1(pTrackerServer, NULL,
            file_buff, file_size, master_file_id, prefix_name, file_ext_name,
            NULL, 0, file_id);

    logDebug("master_filename:%s, prefix_name:%s, file_ext_name:%s, file_id:%s",
            master_file_id, prefix_name, file_ext_name, file_id);

    if (result != 0) {
        logError("CFDFSClient::upload_slave() storage_upload_slave_by_filebuff1 fail,"
                " file_id:%s, error no: %d, error info: %s",
                file_id, result, STRERROR(result));
    } else {
        name_size = snprintf(m_pRemoteFileName, REMOTE_FILE_NAME_MAX_SIZE, "%s", file_id);
        remote_file_id = m_pRemoteFileName;
    }

    tracker_close_connection_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::upload_appender(const char *file_buff, int64_t file_size, const char *file_ext_name,
        char *&remote_file_name, int &name_size) {
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::upload_appender() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
    char remote_filename[BUFF_SIZE];

    int store_path_index;
    ConnectionInfo storageServer;

    if ((result = tracker_query_storage_store(pTrackerServer, &storageServer,
            group_name, &store_path_index)) != 0) {
        tracker_close_connection_ex(pTrackerServer, true);

        logError("CFDFSClient::upload_appender() tracker_query_storage_store"
                " fail, error no: %d, error info: %s", result, STRERROR(result));
        return result;
    }

    if (file_ext_name != NULL && (result = strlen(file_ext_name)) > FDFS_FILE_EXT_NAME_MAX_LEN) {
        logWarning("CFDFSClient::upload_appender() : file ext name length "
                "should <= %d !, however got %d : %s",
                FDFS_FILE_EXT_NAME_MAX_LEN, result, file_ext_name);
    }

    result = storage_upload_appender_by_filebuff(pTrackerServer, &storageServer,
            store_path_index, file_buff, file_size, file_ext_name,
            NULL, 0, group_name, remote_filename);
    if (result != 0) {
        logError("CFDFSClient::upload_appender() storage_upload_appender_by_filebuff"
                " fail, group:%s, remote:%s, error no: %d, error info: %s",
                group_name, remote_filename, result, STRERROR(result));
    } else {
        name_size = snprintf(m_pRemoteFileName, REMOTE_FILE_NAME_MAX_SIZE,
                "%s%c%s", group_name, FDFS_FILE_ID_SEPERATOR, remote_filename);
        remote_file_name = m_pRemoteFileName;
    }

    tracker_close_connection_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::append_file(const char *file_buff, const int64_t file_size,
        const char *appender_file_id) {
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::append_file() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    if ((result = storage_append_by_filebuff1(pTrackerServer, NULL,
            file_buff, file_size, appender_file_id)) != 0) {
        logError("CFDFSClient::append_file() storage_append_by_filebuff1 fail,"
                " appender_file_id:%s, error no: %d, error info: %s",
                appender_file_id, result, STRERROR(result));
    }

    tracker_close_connection_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::download_file(const char *group_name, const char *remote_filename,
        BufferInfo *pBuff) {
    char *file_buff = NULL;
    int64_t file_size = 0;
    int result = 0;

    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::download_file() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    result = storage_download_file_to_buff(pTrackerServer, NULL,
            group_name, remote_filename, &file_buff, &file_size);
    if (result != 0) {
        logError("CFDFSClient::download_file() storage_download_file_to_buff fail,"
                " group:%s, remote:%s, error no: %d, error info: %s",
                group_name, remote_filename, result, STRERROR(result));
        result = FSC_ERROR_CODE_DOWNLAOD_FILE_FAIL;
    } else {
        buffer_memcpy(&m_RecvBufferInfo, file_buff, file_size);
        *pBuff = m_RecvBufferInfo;
        free(file_buff);
    }

    tracker_close_connection_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::delete_file(const char *group_name, const char *remote_filename) {
    int result = 0;

    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::delete_file() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    if ((result = storage_delete_file(pTrackerServer, NULL,
            group_name, remote_filename)) != 0) {
        logError("CFDFSClient::delete_file() storage_delete_file fail,"
                " group:%s, remote:%s, error no: %d, error info: %s",
                group_name, remote_filename, result, STRERROR(result));

        result = FSC_ERROR_CODE_DELETE_FILE_FAIL;
    }

    tracker_close_connection_ex(pTrackerServer, true);

    return result;
}

int CFDFSClient::get_file_info(const char *file_id, BufferInfo *file_info) {
    int result = 0;
    FDFSFileInfo info;

    if ((result = fdfs_get_file_info1(file_id, &info)) != 0) {
        logError("CFDFSClient::get_file_info() fdfs_get_file_info1 fail,"
                " error no: %d, error info: %s", result, STRERROR(result));
        return result;
    }

    //    tracker_close_all_connections();

    json_map_t info_value;
    char error_info[BUFF_SIZE];

    init_json_map(&info_value, 5);

    assign_json_map(info_value.elements[0], "create_timestamp", "%s",
            formatDatetime(info.create_timestamp,
            "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
    assign_json_map(info_value.elements[1], "crc32",
            "%u", info.crc32);
    assign_json_map(info_value.elements[2], "source_id",
            "%d", info.source_id);
    assign_json_map(info_value.elements[3], "file_size",
            "%" PRId64, info.file_size);
    assign_json_map(info_value.elements[4], "source_ip_addr",
            "%s", info.source_ip_addr);

    string_t output;

    if ((result = encode_json_map(&info_value, &output,
            error_info, sizeof (error_info))) != 0) {
        logError("CFDFSClient::get_file_info() encode_json_map fail,"
                " error no: %d, error info: %s", result, error_info);

        free_json_map(&info_value, info_value.count);

        return result;
    }

    free_json_map(&info_value, info_value.count);

    buffer_memcpy(&m_RecvBufferInfo, output.str, output.len);

    free_json_string(&output);

    *file_info = m_RecvBufferInfo;

    return result;
}

int CFDFSClient::list_groups(BufferInfo *groups_info) {
    int result = 0;
    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::list_groups() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    int group_count;
    FDFSGroupStat group_stats[FDFS_MAX_GROUPS];
    result = tracker_list_groups(pTrackerServer,
            group_stats, FDFS_MAX_GROUPS, &group_count);

    tracker_close_connection_ex(pTrackerServer, true);

    if (result != 0) {
        logError("CFDFSClient::list_groups() tracker_list_groups fail,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return result;
    }

    json_array_t groups_value;
    json_map_t info;
    char error_info[BUFF_SIZE];

    init_json_array(&groups_value, group_count);
    init_json_map(&info, 12);

    for (int n = 0; n < group_count; ++n) {
        assign_json_map(info.elements[0], "group_name",
                "%s", group_stats[n].group_name);
        assign_json_map(info.elements[1], "total_mb",
                "%" PRId64, group_stats[n].total_mb);
        assign_json_map(info.elements[2], "free_mb",
                "%" PRId64, group_stats[n].free_mb);
        assign_json_map(info.elements[3], "trunk_free_mb",
                "%" PRId64, group_stats[n].trunk_free_mb);
        assign_json_map(info.elements[4], "count",
                "%d", group_stats[n].count);
        assign_json_map(info.elements[5], "storage_port",
                "%d", group_stats[n].storage_port);
        assign_json_map(info.elements[6], "storage_http_port",
                "%d", group_stats[n].storage_http_port);
        assign_json_map(info.elements[7], "active_count",
                "%d", group_stats[n].active_count);
        assign_json_map(info.elements[8], "current_write_server",
                "%d", group_stats[n].current_write_server);
        assign_json_map(info.elements[9], "store_path_count",
                "%d", group_stats[n].store_path_count);
        assign_json_map(info.elements[10], "subdir_count_per_path",
                "%d", group_stats[n].subdir_count_per_path);
        assign_json_map(info.elements[11], "current_trunk_file_id",
                "%d", group_stats[n].current_trunk_file_id);

        if ((result = encode_json_map(&info, &groups_value.elements[n],
                error_info, sizeof (error_info))) != 0) {
            logError("CFDFSClient::list_groups() encode_json_map fail,"
                    " error no: %d, error info: %s", result, error_info);

            free_json_map(&info, info.count);
            free_json_array(&groups_value, n);

            return result;
        }
    }

    free_json_map(&info, info.count);

    string_t output;
    result = encode_json_array(&groups_value, &output,
            error_info, sizeof (error_info));

    free_json_array(&groups_value, group_count);

    if (result != 0) {
        logError("CFDFSClient::list_groups() encode_json_array fail,"
                " error no: %d, error info: %s", result, error_info);
        return result;
    }

    buffer_memcpy(&m_RecvBufferInfo, output.str, output.len);

    free_json_string(&output);

    *groups_info = m_RecvBufferInfo;

    return result;
}

int CFDFSClient::list_one_group(const char *group_name, BufferInfo *group_info) {
    int result;
    FDFSGroupStat group_stat;

    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::list_one_group() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    result = tracker_list_one_group(pTrackerServer, group_name, &group_stat);

    tracker_close_connection_ex(pTrackerServer, true);

    if (result != 0) {
        logError("CFDFSClient::list_one_group() tracker_list_one_group fail,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return result;
    }

    json_map_t groups_value;
    char error_info[BUFF_SIZE];

    init_json_map(&groups_value, 12);

    assign_json_map(groups_value.elements[0], "group_name",
            "%s", group_stat.group_name);
    assign_json_map(groups_value.elements[1], "total_mb",
            "%" PRId64, group_stat.total_mb);
    assign_json_map(groups_value.elements[2], "free_mb",
            "%" PRId64, group_stat.free_mb);
    assign_json_map(groups_value.elements[3], "trunk_free_mb",
            "%" PRId64, group_stat.trunk_free_mb);
    assign_json_map(groups_value.elements[4], "count",
            "%d", group_stat.count);
    assign_json_map(groups_value.elements[5], "storage_port",
            "%d", group_stat.storage_port);
    assign_json_map(groups_value.elements[6], "storage_http_port",
            "%d", group_stat.storage_http_port);
    assign_json_map(groups_value.elements[7], "active_count",
            "%d", group_stat.active_count);
    assign_json_map(groups_value.elements[8], "current_write_server",
            "%d", group_stat.current_write_server);
    assign_json_map(groups_value.elements[9], "store_path_count",
            "%d", group_stat.store_path_count);
    assign_json_map(groups_value.elements[10], "subdir_count_per_path",
            "%d", group_stat.subdir_count_per_path);
    assign_json_map(groups_value.elements[11], "current_trunk_file_id",
            "%d", group_stat.current_trunk_file_id);

    string_t output;

    if ((result = encode_json_map(&groups_value, &output,
            error_info, sizeof (error_info))) != 0) {
        logError("CFDFSClient::list_one_group() encode_json_map fail,"
                " error no: %d, error info: %s", result, error_info);

        free_json_map(&groups_value, groups_value.count);

        return result;
    }

    free_json_map(&groups_value, groups_value.count);

    buffer_memcpy(&m_RecvBufferInfo, output.str, output.len);

    free_json_string(&output);

    *group_info = m_RecvBufferInfo;

    return result;
}

int CFDFSClient::list_servers(const char *group_name, const char *storage_id,
        BufferInfo *storages_info) {
    int result;
    int storage_count;
    FDFSStorageInfo storage_infos[FDFS_MAX_SERVERS_EACH_GROUP];
    FDFSStorageStat *pStorageStat;

    ConnectionInfo *pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        result = (errno != 0 ? errno : ECONNREFUSED);
        logError("CFDFSClient::list_servers() tracker_get_connection is failed,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return FSC_ERROR_CODE_CONNECT_TRACKER_FAIL;
    }

    result = tracker_list_servers(pTrackerServer, group_name, storage_id,
            storage_infos, FDFS_MAX_SERVERS_EACH_GROUP, &storage_count);

    tracker_close_connection_ex(pTrackerServer, true);

    if (result != 0) {
        logError("CFDFSClient::list_servers() tracker_list_servers fail,"
                " error no: %d, error info: %s", result, STRERROR(result));

        return result;
    }

    json_array_t storages_value;
    json_map_t info;
    char error_info[BUFF_SIZE];

    init_json_array(&storages_value, storage_count);
    init_json_map(&info, 62);

    for (int n = 0; n < storage_count; ++n) {
        assign_json_map(info.elements[0], "if_trunk_server",
                "%d", storage_infos[n].if_trunk_server);
        assign_json_map(info.elements[1], "status",
                "%s", get_storage_status_caption(storage_infos[n].status));
        assign_json_map(info.elements[2], "id",
                "%s", storage_infos[n].id);
        assign_json_map(info.elements[3], "ip_addr",
                "%s", storage_infos[n].ip_addr);
        assign_json_map(info.elements[4], "src_id",
                "%s", storage_infos[n].src_id);
        assign_json_map(info.elements[5], "domain_name",
                "%s", storage_infos[n].domain_name);
        assign_json_map(info.elements[6], "version",
                "%s", storage_infos[n].version);
        assign_json_map(info.elements[7], "total_mb",
                "%d", storage_infos[n].total_mb);
        assign_json_map(info.elements[8], "free_mb",
                "%d", storage_infos[n].free_mb);
        assign_json_map(info.elements[9], "upload_priority",
                "%d", storage_infos[n].upload_priority);
        assign_json_map(info.elements[10], "join_time", "%s",
                formatDatetime(storage_infos[n].join_time,
                "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
        assign_json_map(info.elements[11], "up_time", "%s",
                formatDatetime(storage_infos[n].up_time,
                "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
        assign_json_map(info.elements[12], "store_path_count",
                "%d", storage_infos[n].store_path_count);
        assign_json_map(info.elements[13], "subdir_count_per_path",
                "%d", storage_infos[n].subdir_count_per_path);
        assign_json_map(info.elements[14], "storage_port",
                "%d", storage_infos[n].storage_port);
        assign_json_map(info.elements[15], "storage_http_port",
                "%d", storage_infos[n].storage_http_port);
        assign_json_map(info.elements[16], "current_write_path",
                "%d", storage_infos[n].current_write_path);

        // 统计数据
        pStorageStat = &(storage_infos[n].stat);

        assign_json_map(info.elements[17], "total_upload_count",
                "%" PRId64, pStorageStat->total_upload_count);
        assign_json_map(info.elements[18], "success_upload_count",
                "%" PRId64, pStorageStat->success_upload_count);
        assign_json_map(info.elements[19], "total_append_count",
                "%" PRId64, pStorageStat->total_append_count);
        assign_json_map(info.elements[20], "success_append_count",
                "%" PRId64, pStorageStat->success_append_count);
        assign_json_map(info.elements[21], "total_modify_count",
                "%" PRId64, pStorageStat->total_modify_count);
        assign_json_map(info.elements[22], "success_modify_count",
                "%" PRId64, pStorageStat->success_modify_count);
        assign_json_map(info.elements[23], "total_truncate_count",
                "%" PRId64, pStorageStat->total_truncate_count);
        assign_json_map(info.elements[24], "success_truncate_count",
                "%" PRId64, pStorageStat->success_truncate_count);
        assign_json_map(info.elements[25], "total_set_meta_count",
                "%" PRId64, pStorageStat->total_set_meta_count);
        assign_json_map(info.elements[26], "success_set_meta_count",
                "%" PRId64, pStorageStat->success_set_meta_count);
        assign_json_map(info.elements[27], "total_delete_count",
                "%" PRId64, pStorageStat->total_delete_count);
        assign_json_map(info.elements[28], "success_delete_count",
                "%" PRId64, pStorageStat->success_delete_count);
        assign_json_map(info.elements[29], "total_download_count",
                "%" PRId64, pStorageStat->total_download_count);
        assign_json_map(info.elements[30], "success_download_count",
                "%" PRId64, pStorageStat->success_download_count);
        assign_json_map(info.elements[31], "total_get_meta_count",
                "%" PRId64, pStorageStat->total_get_meta_count);
        assign_json_map(info.elements[32], "success_get_meta_count",
                "%" PRId64, pStorageStat->success_get_meta_count);
        assign_json_map(info.elements[33], "total_create_link_count",
                "%" PRId64, pStorageStat->total_create_link_count);
        assign_json_map(info.elements[34], "success_create_link_count",
                "%" PRId64, pStorageStat->success_create_link_count);
        assign_json_map(info.elements[35], "total_delete_link_count",
                "%" PRId64, pStorageStat->total_delete_link_count);
        assign_json_map(info.elements[36], "success_delete_link_count",
                "%" PRId64, pStorageStat->success_delete_link_count);
        assign_json_map(info.elements[37], "total_upload_bytes",
                "%" PRId64, pStorageStat->total_upload_bytes);
        assign_json_map(info.elements[38], "success_upload_bytes",
                "%" PRId64, pStorageStat->success_upload_bytes);
        assign_json_map(info.elements[39], "total_append_bytes",
                "%" PRId64, pStorageStat->total_append_bytes);
        assign_json_map(info.elements[40], "success_append_bytes",
                "%" PRId64, pStorageStat->success_append_bytes);
        assign_json_map(info.elements[41], "total_modify_bytes",
                "%" PRId64, pStorageStat->total_modify_bytes);
        assign_json_map(info.elements[42], "success_modify_bytes",
                "%" PRId64, pStorageStat->success_modify_bytes);
        assign_json_map(info.elements[43], "total_download_bytes",
                "%" PRId64, pStorageStat->total_download_bytes);
        assign_json_map(info.elements[44], "success_download_bytes",
                "%" PRId64, pStorageStat->success_download_bytes);
        assign_json_map(info.elements[45], "total_sync_in_bytes",
                "%" PRId64, pStorageStat->total_sync_in_bytes);
        assign_json_map(info.elements[46], "success_sync_in_bytes",
                "%" PRId64, pStorageStat->success_sync_in_bytes);
        assign_json_map(info.elements[47], "total_sync_out_bytes",
                "%" PRId64, pStorageStat->total_sync_out_bytes);
        assign_json_map(info.elements[48], "success_sync_out_bytes",
                "%" PRId64, pStorageStat->success_sync_out_bytes);
        assign_json_map(info.elements[49], "total_file_open_count",
                "%" PRId64, pStorageStat->total_file_open_count);
        assign_json_map(info.elements[50], "success_file_open_count",
                "%" PRId64, pStorageStat->success_file_open_count);
        assign_json_map(info.elements[51], "total_file_read_count",
                "%" PRId64, pStorageStat->total_file_read_count);
        assign_json_map(info.elements[52], "success_file_read_count",
                "%" PRId64, pStorageStat->success_file_read_count);
        assign_json_map(info.elements[53], "total_file_write_count",
                "%" PRId64, pStorageStat->total_file_write_count);
        assign_json_map(info.elements[54], "success_file_write_count",
                "%" PRId64, pStorageStat->success_file_write_count);
        assign_json_map(info.elements[55], "last_source_update", "%s",
                formatDatetime(pStorageStat->last_source_update,
                "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
        assign_json_map(info.elements[56], "last_sync_update", "%s",
                formatDatetime(pStorageStat->last_sync_update,
                "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
        assign_json_map(info.elements[57], "last_synced_timestamp", "%s",
                formatDatetime(pStorageStat->last_synced_timestamp,
                "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
        assign_json_map(info.elements[58], "last_heart_beat_time", "%s",
                formatDatetime(pStorageStat->last_heart_beat_time,
                "%Y-%m-%d %H:%M:%S", NULL, BUFF_SIZE));
        assign_json_map(info.elements[59], "connection.alloc_count",
                "%d", pStorageStat->connection.alloc_count);
        assign_json_map(info.elements[60], "connection.current_count",
                "%d", pStorageStat->connection.current_count);
        assign_json_map(info.elements[61], "connection.max_count",
                "%d", pStorageStat->connection.max_count);

        if ((result = encode_json_map(&info, &storages_value.elements[n],
                error_info, sizeof (error_info))) != 0) {
            logError("CFDFSClient::list_servers() encode_json_map fail,"
                    " error no: %d, error info: %s", result, error_info);

            free_json_map(&info, info.count);
            free_json_array(&storages_value, n);

            return result;
        }
    }

    free_json_map(&info, info.count);

    string_t output;
    result = encode_json_array(&storages_value, &output,
            error_info, sizeof (error_info));

    free_json_array(&storages_value, storage_count);

    if (result != 0) {
        logError("CFDFSClient::list_servers() encode_json_array fail,"
                " error no: %d, error info: %s", result, error_info);
        return result;
    }

    buffer_memcpy(&m_RecvBufferInfo, output.str, output.len);

    free_json_string(&output);

    *storages_info = m_RecvBufferInfo;

    return result;
}
