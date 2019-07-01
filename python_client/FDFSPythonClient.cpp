#include "FDFSClient.h"
#include "FDFSPythonClient.h"

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

    int nRes = 0;
    nRes = init(sConfig, nLogLevel, nLogFD, bLogTakeOverStd);
    return Py_BuildValue("i", nRes);
}

static PyObject *wrap_destory(PyObject *self, PyObject *args) {
    int nRes = destory();
    return Py_BuildValue("i", nRes);
}

static PyObject *wrap_upload_file(PyObject *self, PyObject *args) {
    const char *sFileContent;
    const char *sFileExtName;
    int nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#s", &sFileContent, &nFileSize, &sFileExtName))
        return NULL;

    int nNameSize = 0;
    char *pRemoteFileName;
    int res = upload_file(sFileContent, sFileExtName, nFileSize, nNameSize,
            pRemoteFileName);
    return Py_BuildValue("(i, s#)", res, pRemoteFileName, nNameSize);
}

static PyObject *wrap_upload_appender(PyObject *self, PyObject *args) {
    const char *sFileContent;
    const char *sFileExtName;
    int nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#s", &sFileContent, &nFileSize, &sFileExtName))
        return NULL;

    int nNameSize = 0;
    char *pRemoteFileName;
    int res = upload_appender(sFileContent, sFileExtName, nFileSize, nNameSize,
            pRemoteFileName);
    return Py_BuildValue("(i, s#)", res, pRemoteFileName, nNameSize);
}

static PyObject *wrap_append_file(PyObject *self, PyObject *args) {
    const char *sFileBuff;
    const char *sAppenderFilename;
    int nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#s", &sFileBuff, &nFileSize, &sAppenderFilename))
        return NULL;

    int res = append_file(sFileBuff, nFileSize, sAppenderFilename);
    return Py_BuildValue("i", res);
}

static PyObject *wrap_upload_slave(PyObject *self, PyObject *args) {
    const char *sFileContent;
    const char *sFileExtName;
    const char *sMasterFileName;
    const char *sPrefixName;
    int nFileSize = 0;
    if (!PyArg_ParseTuple(args, "s#sss", &sFileContent, &nFileSize,
            &sFileExtName, &sMasterFileName, &sPrefixName))
        return NULL;

    int nNameSize = 0;
    char *pRemoteFileName;
    int res = upload_slave(sFileContent, sMasterFileName, sPrefixName,
            sFileExtName, nFileSize, nNameSize, pRemoteFileName);
    return Py_BuildValue("(i, s#)", res, pRemoteFileName, nNameSize);
}

static PyObject *wrap_download_file(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sRomteName;
    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sRomteName))
        return NULL;

    BufferInfo tgBuff = {0};
    int res = download_file(&tgBuff, sGroupName, sRomteName);
#if PY_MAJOR_VERSION >= 3
    return Py_BuildValue("(i, y#)", res, tgBuff.buff, tgBuff.length);
#else
    return Py_BuildValue("(i, s#)", res, tgBuff.buff, tgBuff.length);
#endif
}

static PyObject *wrap_delete_file(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sRomteName;
    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sRomteName))
        return NULL;

    int res = delete_file(sGroupName, sRomteName);
    return Py_BuildValue("i", res);
}

static PyObject *wrap_list_all_groups(PyObject *self, PyObject *args) {
    BufferInfo GroupsInfo = {0};
    int res = list_all_groups(&GroupsInfo);
    return Py_BuildValue("(i,s#)", res, GroupsInfo.buff, GroupsInfo.length);
}

static PyObject *wrap_list_one_group(PyObject *self, PyObject *args) {
    const char *sGroupName;
    if (!PyArg_ParseTuple(args, "s", &sGroupName))
        return NULL;

    BufferInfo GroupsInfo = {0};
    int res = list_one_group(sGroupName, &GroupsInfo);
    return Py_BuildValue("(i,s#)", res, GroupsInfo.buff, GroupsInfo.length);
}

static PyObject *wrap_list_storages(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sStorageID;
    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sStorageID))
        return NULL;

    BufferInfo StoragesInfo = {0};
    int res = list_storages(sGroupName, sStorageID, &StoragesInfo);
    return Py_BuildValue("(i,s#)", res, StoragesInfo.buff, StoragesInfo.length);
}

// 方法列表
static PyMethodDef FDFSMethods[] = {
    //python中注册的函数名
    {"init", (PyCFunction) wrap_init, METH_VARARGS | METH_KEYWORDS, NULL},
    {"destory", wrap_destory, METH_VARARGS, NULL},
    {"upload_file", wrap_upload_file, METH_VARARGS, NULL},
    {"upload_appender", wrap_upload_appender, METH_VARARGS, NULL},
    {"append_file", wrap_append_file, METH_VARARGS, NULL},
    {"upload_slave", wrap_upload_slave, METH_VARARGS, NULL},
    {"download_file", wrap_download_file, METH_VARARGS, NULL},
    {"delete_file", wrap_delete_file, METH_VARARGS, NULL},
    {"list_all_groups", wrap_list_all_groups, METH_VARARGS, NULL},
    {"list_one_group", wrap_list_one_group, METH_VARARGS, NULL},
    {"list_storages", wrap_list_storages, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

// 模块初始化方法
#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef FDFSModuledef = {
    PyModuleDef_HEAD_INIT,
    "FDFSPythonClient",
    "FastDFS Client for Python",
    -1,
    FDFSMethods
};

PyMODINIT_FUNC PyInit_FDFSPythonClient(void) {
    //初始模块
    PyObject *m = PyModule_Create(&FDFSModuledef);
    if (m == NULL)
        INITERROR;
    return m;
}
#else

PyMODINIT_FUNC initFDFSPythonClient(void) {
    //初始模块
    PyObject *m = Py_InitModule("FDFSPythonClient", FDFSMethods);
    if (m == NULL)
        INITERROR;
    return;
}
#endif

/////////////////////////////////// 接口实现 ///////////////////////////////////
CFDFSClient *g_pClient = NULL;

int init(const char *sConfig,
        int nLogLevel, int nLogFD, bool bLogTakeOverStd) {
    destory();
    g_pClient = new CFDFSClient();

    int nResult = g_pClient->init(sConfig, nLogLevel, nLogFD, bLogTakeOverStd);
    if (nResult != 0)
        destory();

    return nResult;
}

int destory() {
    if (g_pClient != NULL) {
        delete g_pClient;
        g_pClient = NULL;
    }
    return 0;
}

int upload_file(const char *file_buff, const char *file_ext_name,
        int file_size, int &name_size, char *&remote_file_name) {
    if (file_buff == NULL || file_size == 0) {
        logDebug("tt:%d", file_size);
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->upload_file(file_buff,
            file_ext_name, file_size, name_size, remote_file_name);

    return result;
}

int upload_appender(const char *file_buff, const char *file_ext_name,
        int file_size, int &name_size, char *&remote_file_name) {
    if (file_buff == NULL || file_size == 0) {
        logDebug("tt:%d", file_size);
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->upload_appender(file_buff,
            file_ext_name, file_size, name_size, remote_file_name);

    return result;
}

int append_file(const char *file_buff, int file_size,
        const char *appender_filename) {
    if (file_buff == NULL || file_size == 0) {
        logDebug("tt:%d", file_size);
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->append_file(file_buff, file_size, appender_filename);

    return result;
}

int upload_slave(const char *file_buff, const char *master_filename,
        const char *prefix_name, const char *file_ext_name, int file_size,
        int &name_size, char *&remote_file_name) {
    if (file_buff == NULL || file_size == 0) {
        logDebug("tt:%d", file_size);
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->upload_slave(file_buff, master_filename,
            prefix_name, file_ext_name, file_size, name_size, remote_file_name);

    return result;
}

int download_file(BufferInfo *pBuff,
        const char *group_name, const char *remote_filename) {
    if (group_name == NULL || remote_filename == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->download_file(pBuff, group_name, remote_filename);

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

int list_all_groups(BufferInfo *group_info) {
    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->list_all_groups(group_info);

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

int list_storages(const char *group_name,
        const char *storage_id,
        BufferInfo *storages_info) {
    if (group_name == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (strcmp(storage_id, "") == 0) {
        storage_id = NULL;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
    result = g_pClient->list_storages(group_name, storage_id, storages_info);

    return result;
}
