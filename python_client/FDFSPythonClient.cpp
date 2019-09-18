#include "FDFSPythonClient.h"

#define PY_SSIZE_T_CLEAN  /* Make "s#" use Py_ssize_t rather than int. */
#include <Python.h>

#include "FDFSClient.h"

////////////////////////////// Python 与 C++ 接口 //////////////////////////////

static PyObject *wrap_init(PyObject *self, PyObject *args, PyObject *kw) {
    static char *keywords[] = {
        (char *) "config", (char *) "log_level", (char *) "log_fd",
        (char *) "take_over_std", NULL
    };
    const char *sConfig;
    int nLogLevel = -1;
    int nLogFD = -1;
    bool bLogTakeOverStd = false;

    if (!PyArg_ParseTupleAndKeywords(args, kw, "s|iip", keywords, &sConfig,
            &nLogLevel, &nLogFD, &bLogTakeOverStd))
        return NULL;

    int nRes = init(sConfig, nLogLevel, nLogFD, bLogTakeOverStd);
    return Py_BuildValue("i", nRes);
}

static PyObject *wrap_destroy(PyObject *self, PyObject *args) {
    int nRes = destroy();
    return Py_BuildValue("i", nRes);
}

static PyObject *wrap_upload_file(PyObject *self, PyObject *args) {
    const char *sFileContent;
    const char *sFileExtName;
    Py_ssize_t nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#s", &sFileContent, &nFileSize, &sFileExtName))
        return NULL;

    char *pRemoteFilename;
    Py_ssize_t nNameSize = 0;
    int res = upload_file(sFileContent, nFileSize, sFileExtName,
            pRemoteFilename, (int &) nNameSize);
    return Py_BuildValue("(i, s#)", res, pRemoteFilename, nNameSize);
}

static PyObject *wrap_upload_appender(PyObject *self, PyObject *args) {
    const char *sFileContent;
    const char *sFileExtName;
    Py_ssize_t nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#s", &sFileContent, &nFileSize, &sFileExtName))
        return NULL;

    char *pRemoteFilename;
    Py_ssize_t nNameSize = 0;
    int res = upload_appender(sFileContent, nFileSize, sFileExtName,
            pRemoteFilename, (int &) nNameSize);
    return Py_BuildValue("(i, s#)", res, pRemoteFilename, nNameSize);
}

static PyObject *wrap_append_file(PyObject *self, PyObject *args) {
    const char *sFileBuff;
    const char *sAppenderFilename;
    Py_ssize_t nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#s", &sFileBuff, &nFileSize, &sAppenderFilename))
        return NULL;

    int res = append_file(sFileBuff, nFileSize, sAppenderFilename);
    return Py_BuildValue("i", res);
}

static PyObject *wrap_upload_slave(PyObject *self, PyObject *args) {
    const char *sFileContent;
    const char *sMasterFilename;
    const char *sPrefixName;
    const char *sFileExtName;
    Py_ssize_t nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#sss", &sFileContent, &nFileSize,
            &sMasterFilename, &sPrefixName, &sFileExtName))
        return NULL;

    char *pRemoteFilename;
    Py_ssize_t nNameSize = 0;
    int res = upload_slave(sFileContent, nFileSize,
            sMasterFilename, sPrefixName, sFileExtName,
            pRemoteFilename, (int &) nNameSize);
    return Py_BuildValue("(i, s#)", res, pRemoteFilename, nNameSize);
}

static PyObject *wrap_download_file(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sReomteFilename;
    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sReomteFilename))
        return NULL;

    BufferInfo tgBuff = {0};
    int res = download_file(sGroupName, sReomteFilename, &tgBuff);

#if PY_MAJOR_VERSION >= 3
    return Py_BuildValue("(i, y#)", res, tgBuff.buff, tgBuff.length);
#else
    return Py_BuildValue("(i, s#)", res, tgBuff.buff, tgBuff.length);
#endif
}

static PyObject *wrap_delete_file(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sReomteFilename;
    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sReomteFilename))
        return NULL;

    int res = delete_file(sGroupName, sReomteFilename);
    return Py_BuildValue("i", res);
}

static PyObject *wrap_get_file_info(PyObject *self, PyObject *args) {
    const char *sFileID;
    if (!PyArg_ParseTuple(args, "s", &sFileID))
        return NULL;

    BufferInfo FileInfo = {0};
    int res = get_file_info(sFileID, &FileInfo);

    return Py_BuildValue("(i,s#)", res, FileInfo.buff, FileInfo.length);
}

static PyObject *wrap_list_groups(PyObject *self, PyObject *args) {
    BufferInfo GroupsInfo = {0};
    int res = list_groups(&GroupsInfo);
    return Py_BuildValue("(i,s#)", res, GroupsInfo.buff, GroupsInfo.length);
}

static PyObject *wrap_list_one_group(PyObject *self, PyObject *args) {
    const char *sGroupName;
    if (!PyArg_ParseTuple(args, "s", &sGroupName))
        return NULL;

    BufferInfo GroupInfo = {0};
    int res = list_one_group(sGroupName, &GroupInfo);
    return Py_BuildValue("(i,s#)", res, GroupInfo.buff, GroupInfo.length);
}

static PyObject *wrap_list_servers(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sStorageID;
    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sStorageID))
        return NULL;

    BufferInfo StoragesInfos = {0};
    int res = list_servers(sGroupName, sStorageID, &StoragesInfos);
    return Py_BuildValue("(i,s#)", res, StoragesInfos.buff, StoragesInfos.length);
}

// 方法列表
static PyMethodDef FDFSMethods[] = {
    //python中注册的函数名
    {"init", (PyCFunction) wrap_init, METH_VARARGS | METH_KEYWORDS, NULL},
    {"destroy", wrap_destroy, METH_VARARGS, NULL},
    {"upload_file", wrap_upload_file, METH_VARARGS, NULL},
    {"upload_appender", wrap_upload_appender, METH_VARARGS, NULL},
    {"append_file", wrap_append_file, METH_VARARGS, NULL},
    {"upload_slave", wrap_upload_slave, METH_VARARGS, NULL},
    {"download_file", wrap_download_file, METH_VARARGS, NULL},
    {"delete_file", wrap_delete_file, METH_VARARGS, NULL},
    {"get_file_info", wrap_get_file_info, METH_VARARGS, NULL},
    {"list_groups", wrap_list_groups, METH_VARARGS, NULL},
    {"list_one_group", wrap_list_one_group, METH_VARARGS, NULL},
    {"list_servers", wrap_list_servers, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

// 模块初始化方法
#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef FDFSModuleDef = {
    PyModuleDef_HEAD_INIT,
    "FDFSPythonClient",
    "FastDFS Client for Python",
    -1,
    FDFSMethods
};

PyMODINIT_FUNC PyInit_FDFSPythonClient(void) {
    return PyModule_Create(&FDFSModuleDef);
}
#else

PyMODINIT_FUNC initFDFSPythonClient(void) {
    (void) Py_InitModule("FDFSPythonClient", FDFSMethods);
}
#endif

/////////////////////////////////// 接口实现 ///////////////////////////////////
CFDFSClient *g_pClient = NULL;

int init(const char *sConfig,
        int nLogLevel, int nLogFD, bool bLogTakeOverStd) {
    destroy();
    g_pClient = new CFDFSClient();

    int nResult = g_pClient->init(sConfig, nLogLevel, nLogFD, bLogTakeOverStd);
    if (nResult != 0)
        destroy();

    return nResult;
}

int destroy() {
    if (g_pClient != NULL) {
        delete g_pClient;
        g_pClient = NULL;
    }
    return 0;
}

int upload_file(const char *file_buff, int64_t file_size,
        const char *file_ext_name, char *&remote_file_name, int &name_size) {
    if (file_buff == NULL || file_size == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->upload_file(file_buff, file_size,
            file_ext_name, remote_file_name, name_size);

    return result;
}

int upload_appender(const char *file_buff, int64_t file_size,
        const char *file_ext_name, char *&remote_file_name, int &name_size) {
    if (file_buff == NULL || file_size == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->upload_appender(file_buff, file_size, file_ext_name,
            remote_file_name, name_size);

    return result;
}

int append_file(const char *file_buff, int64_t file_size,
        const char *appender_filename) {
    if (file_buff == NULL || file_size == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->append_file(file_buff, file_size, appender_filename);

    return result;
}

int upload_slave(const char *file_buff, int64_t file_size,
        const char *master_filename, const char *prefix_name,
        const char *file_ext_name, char *&remote_file_name, int &name_size) {
    if (file_buff == NULL || file_size == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->upload_slave(file_buff, file_size,
            master_filename, prefix_name, file_ext_name,
            remote_file_name, name_size);

    return result;
}

int download_file(const char *group_name, const char *remote_filename,
        BufferInfo *pBuff) {
    if (group_name == NULL || remote_filename == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->download_file(group_name, remote_filename, pBuff);

    return result;
}

int delete_file(const char *group_name, const char *remote_filename) {
    if (group_name == NULL || remote_filename == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->delete_file(group_name, remote_filename);

    return result;
}

int get_file_info(const char *sFileID, BufferInfo *file_info) {
    if (sFileID == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = g_pClient->get_file_info(sFileID, file_info);

    return result;
}

int list_groups(BufferInfo *group_info) {
    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->list_groups(group_info);

    return result;
}

int list_one_group(const char *group_name, BufferInfo *group_info) {
    if (group_name == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->list_one_group(group_name, group_info);

    return result;
}

int list_servers(const char *group_name, const char *storage_id,
        BufferInfo *storages_infos) {
    if (group_name == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (strcmp(storage_id, "") == 0) {
        storage_id = NULL;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->list_servers(group_name, storage_id, storages_infos);

    return result;
}
