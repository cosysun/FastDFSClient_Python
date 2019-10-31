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

    if (!PyArg_ParseTupleAndKeywords(args, kw, "s|iip", keywords,
            &sConfig, &nLogLevel, &nLogFD, &bLogTakeOverStd))
        return NULL;

    int nRes = init(sConfig, nLogLevel, nLogFD, bLogTakeOverStd);

    return Py_BuildValue("i", nRes);
}

static PyObject *wrap_destroy(PyObject *self, PyObject *args) {
    return Py_BuildValue("i", destroy());
}

static PyObject *wrap_upload_file(PyObject *self, PyObject *args) {
    const char *sFileBuff;
    const char *sFileExtName;
    Py_ssize_t nFileSize = 0;

    if (!PyArg_ParseTuple(args, "s#s", &sFileBuff, &nFileSize, &sFileExtName))
        return NULL;

    char *pRemoteFilename;
    Py_ssize_t nNameSize = 0;

    int res = upload_file(sFileBuff, nFileSize, sFileExtName,
            pRemoteFilename, (int &) nNameSize);

    return Py_BuildValue("(i, s#)", res, pRemoteFilename, nNameSize);
}

static PyObject *wrap_upload_slave(PyObject *self, PyObject *args) {
    const char *sFileBuff;
    const char *sMasterFilename;
    const char *sPrefixName;
    const char *sFileExtName;
    Py_ssize_t nFileSize = 0;

    if (!PyArg_ParseTuple(args, "s#sss", &sFileBuff, &nFileSize,
            &sMasterFilename, &sPrefixName, &sFileExtName))
        return NULL;

    char *pRemoteFilename;
    Py_ssize_t nNameSize = 0;

    int res = upload_slave(sFileBuff, nFileSize,
            sMasterFilename, sPrefixName, sFileExtName,
            pRemoteFilename, (int &) nNameSize);

    return Py_BuildValue("(i, s#)", res, pRemoteFilename, nNameSize);
}

static PyObject *wrap_upload_appender(PyObject *self, PyObject *args) {
    const char *sFileBuff;
    const char *sFileExtName;
    Py_ssize_t nFileSize = 0;

    if (!PyArg_ParseTuple(args, "s#s", &sFileBuff, &nFileSize, &sFileExtName))
        return NULL;

    char *pRemoteFilename;
    Py_ssize_t nNameSize = 0;

    int res = upload_appender(sFileBuff, nFileSize, sFileExtName,
            pRemoteFilename, (int &) nNameSize);

    return Py_BuildValue("(i, s#)", res, pRemoteFilename, nNameSize);
}

static PyObject *wrap_append_file(PyObject *self, PyObject *args) {
    const char *sFileBuff;
    const char *sAppenderFileID;
    Py_ssize_t nFileSize = 0;

    if (!PyArg_ParseTuple(args, "s#s", &sFileBuff, &nFileSize, &sAppenderFileID))
        return NULL;

    int res = append_file(sFileBuff, nFileSize, sAppenderFileID);

    return Py_BuildValue("i", res);
}

static PyObject *wrap_download_file(PyObject *self, PyObject *args) {
    const char *sGroupName;
    const char *sReomteFilename;

    if (!PyArg_ParseTuple(args, "ss", &sGroupName, &sReomteFilename))
        return NULL;

    BufferInfo pBuff = {0};
    int res = download_file(sGroupName, sReomteFilename, &pBuff);

#if PY_MAJOR_VERSION >= 3
    return Py_BuildValue("(i, y#)", res, pBuff.buff, pBuff.length);
#else
    return Py_BuildValue("(i, s#)", res, pBuff.buff, pBuff.length);
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

    BufferInfo StoragesInfo = {0};
    int res = list_servers(sGroupName, sStorageID, &StoragesInfo);

    return Py_BuildValue("(i,s#)", res, StoragesInfo.buff, StoragesInfo.length);
}

// 方法列表
static PyMethodDef FDFSMethods[] = {
    //python中注册的函数名
    {"init", (PyCFunction) wrap_init, METH_VARARGS | METH_KEYWORDS, NULL},
    {"destroy", wrap_destroy, METH_VARARGS, NULL},
    {"upload_file", wrap_upload_file, METH_VARARGS, NULL},
    {"upload_slave", wrap_upload_slave, METH_VARARGS, NULL},
    {"upload_appender", wrap_upload_appender, METH_VARARGS, NULL},
    {"append_file", wrap_append_file, METH_VARARGS, NULL},
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

int init(const char *sConfig, int nLogLevel, int nLogFD, bool bLogTakeOverStd) {
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

int upload_file(const char *sFileBuff, int64_t nFileSize, const char *sFileExtName,
        char *&pRemoteFilename, int &nNameSize) {
    if (sFileBuff == NULL || nFileSize == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->upload_file(sFileBuff, nFileSize, sFileExtName,
            pRemoteFilename, nNameSize);
}

int upload_slave(const char *sFileBuff, int64_t nFileSize,
        const char *sMasterFilename, const char *sPrefixName, const char *sFileExtName,
        char *&pRemoteFilename, int &nNameSize) {
    if (sFileBuff == NULL || nFileSize == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->upload_slave(sFileBuff, nFileSize,
            sMasterFilename, sPrefixName, sFileExtName,
            pRemoteFilename, nNameSize);
}

int upload_appender(const char *sFileBuff, int64_t nFileSize, const char *sFileExtName,
        char *&pRemoteFilename, int &nNameSize) {
    if (sFileBuff == NULL || nFileSize == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->upload_appender(sFileBuff, nFileSize, sFileExtName,
            pRemoteFilename, nNameSize);
}

int append_file(const char *sFileBuff, const int64_t nFileSize, const char *sAppenderFileID) {
    if (sFileBuff == NULL || nFileSize == 0) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->append_file(sFileBuff, nFileSize,sAppenderFileID);
}

int download_file(const char *sGroupName, const char *sReomteFilename,
        BufferInfo *pBuff) {
    if (sGroupName == NULL || sReomteFilename == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->download_file(sGroupName, sReomteFilename, pBuff);
}

int delete_file(const char *sGroupName, const char *sReomteFilename) {
    if (sGroupName == NULL || sReomteFilename == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->delete_file(sGroupName, sReomteFilename);
}

int get_file_info(const char *sFileID, BufferInfo *FileInfo) {
    if (sFileID == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->get_file_info(sFileID, FileInfo);
}

int list_groups(BufferInfo *GroupsInfo) {
    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->list_groups(GroupsInfo);
}

int list_one_group(const char *sGroupName, BufferInfo *GroupInfo) {
    if (sGroupName == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->list_one_group(sGroupName, GroupInfo);
}

int list_servers(const char *sGroupName, const char *sStorageID,
        BufferInfo *StoragesInfo) {
    if (sGroupName == NULL) {
        return FSC_ERROR_CODE_PARAM_INVAILD;
    }

    if (strcmp(sStorageID, "") == 0) {
        sStorageID = NULL;
    }

    if (g_pClient == NULL)
        return FSC_ERROR_CODE_INIT_FAILED;

    return g_pClient->list_servers(sGroupName, sStorageID, StoragesInfo);
}
