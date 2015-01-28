#include "FDFSPythonClient.h"

//////////////////////////python 与 C++ 接口////////////////////////////////////////////////
static PyObject * wrap_fdfs_init(PyObject *self, PyObject *args) 
{  
	const char* sConfig;
	int nLogLevel = LOG_ERR;
	if (!PyArg_ParseTuple(args, "si", &sConfig, &nLogLevel)) 
		return NULL;

	if (nLogLevel < LOG_EMERG || nLogLevel > LOG_DEBUG)
	{
		nLogLevel = LOG_ERR;
	}
	int nRes = 0;
	nRes = fdfs_init(sConfig, nLogLevel);  
	return Py_BuildValue("i", nRes);

}  

static PyObject * wrap_fdfs_download(PyObject *self, PyObject *args) 
{  
	const char* sGroupName;
	const char* sRomteName;
	if (!PyArg_ParseTuple(args, "ss",  &sGroupName, &sRomteName)) 
		return NULL;  

	BufferInfo tgBuff = {0};
	int res = fdfs_download(&tgBuff, sGroupName, sRomteName); 
    return Py_BuildValue("(i, s#)", res, tgBuff.buff, tgBuff.length);
}  

static PyObject * wrap_fdfs_upload(PyObject *self, PyObject *args) 
{  
	const char* sFileContent;
	const char* sFileExtName;
	int nFileSize = 0;
	if (!PyArg_ParseTuple(args, "s#s",  &sFileContent, &nFileSize, &sFileExtName)) 
		return NULL;  

	int nNameSize = 0;
	char* pRemoteFileName;
	int res = fdfs_upload(sFileContent, sFileExtName, nFileSize, nNameSize, pRemoteFileName); 
    return Py_BuildValue("(i, s#)", res, pRemoteFileName, nNameSize);
} 

static PyObject * wrap_fdfs_delete(PyObject *self, PyObject *args) 
{  
	const char* sGroupName;
	const char* sRomteName;
	if (!PyArg_ParseTuple(args, "ss",  &sGroupName, &sRomteName)) 
		return NULL;  

	int res = fdfs_delete(sGroupName, sRomteName); 
	return Py_BuildValue("i", res);
}  

static PyObject * wrap_fdfs_list_all_groups(PyObject *self, PyObject *args) 
{  
	BufferInfo GroupsInfo = {0};
	int res = list_all_groups(&GroupsInfo); 
	return Py_BuildValue("(i,s#)", res, GroupsInfo.buff, GroupsInfo.length);
} 

static PyObject * wrap_fdfs_list_one_group(PyObject *self, PyObject *args) 
{  
	const char* sGroupName;
	if (!PyArg_ParseTuple(args, "s",  &sGroupName)) 
		return NULL;  

	BufferInfo GroupsInfo = {0};
	int res = list_one_group(sGroupName, &GroupsInfo); 
	return Py_BuildValue("(i,s#)", res, GroupsInfo.buff, GroupsInfo.length);
} 

static PyObject * wrap_list_storages(PyObject *self, PyObject *args) 
{  
	const char* sGroupName;
	const char* sStorageID;
	if (!PyArg_ParseTuple(args, "ss",  &sGroupName, &sStorageID)) 
		return NULL;  

	BufferInfo StoragesInfo = {0};
	int res = list_storages(sGroupName, sStorageID, &StoragesInfo); 
	return Py_BuildValue("(i,s#)", res, StoragesInfo.buff, StoragesInfo.length);
} 

// 方法列表  
static PyMethodDef FDFSMethods[] = {  

	//python中注册的函数名
	{ "fdfs_init", wrap_fdfs_init, METH_VARARGS, "Execute a shell command." },  
	{ "fdfs_download", wrap_fdfs_download, METH_VARARGS, "Execute a shell command." }, 
	{ "fdfs_upload", wrap_fdfs_upload, METH_VARARGS, "Execute a shell command." }, 
	{ "fdfs_delete", wrap_fdfs_delete, METH_VARARGS, "Execute a shell command." }, 
	
	{ "list_all_groups", wrap_fdfs_list_all_groups, METH_VARARGS, "Execute a shell command." }, 
	{ "list_one_group", wrap_fdfs_list_one_group, METH_VARARGS, "Execute a shell command." }, 
	{ "list_storages", wrap_list_storages, METH_VARARGS, "Execute a shell command." }, 
	{ NULL, NULL, 0, NULL }  
};  


// 模块初始化方法  
PyMODINIT_FUNC initFDFSPythonClient(void) {  

	//初始模块
	PyObject *m = Py_InitModule("FDFSPythonClient", FDFSMethods);  
	if (m == NULL)  
		return;  
}  

///////////////////////////接口实现///////////////////////////////////////////////
CFDFSClient * g_pClient = NULL;

int fdfs_init(const char* sConfig, int nLogLevel)
{
	g_pClient = new CFDFSClient();

    int nResult = 0;
	nResult = g_pClient->init(sConfig, nLogLevel);
	if (0 != nResult)
	{
		delete g_pClient;
		g_pClient = NULL;
	}

	return nResult;
}

int fdfs_download(BufferInfo* pBuff, const char *group_name, const char* remote_filename) 
{  
	if (group_name == NULL || remote_filename == NULL)
	{
		return FSC_ERROR_CODE_PARAM_INVAILD;
	}

    if(g_pClient == NULL) return FSC_ERROR_CODE_INIT_FAILED;

    int result = 0;
	result= g_pClient->fdfs_dowloadfile(pBuff, group_name, remote_filename);

    return result;  
}  

int fdfs_upload(const char *file_content, const char *file_ext_name, int file_size, 
				int& name_size, char*& remote_file_name )
{
	if (file_content == NULL || file_size == 0)
	{
		printf("tt:%d", file_size);
        return FSC_ERROR_CODE_PARAM_INVAILD;
	}

	if(g_pClient == NULL) return FSC_ERROR_CODE_INIT_FAILED;

	int result = 0;
	result= g_pClient->fdfs_uploadfile(file_content, file_ext_name, file_size, name_size, remote_file_name);

	return result;  
}

int fdfs_delete(const char *group_name, const char* remote_filename) 
{
	if (group_name == NULL || remote_filename == NULL)
	{
		return FSC_ERROR_CODE_PARAM_INVAILD;
	}

	if(g_pClient == NULL) return FSC_ERROR_CODE_INIT_FAILED;

	int result = 0;
	result= g_pClient->fdfs_deletefile(group_name, remote_filename);

	return result;  
}

int list_all_groups( BufferInfo* group_info )
{
	if(g_pClient == NULL) return FSC_ERROR_CODE_INIT_FAILED;

	int result = 0;
	result= g_pClient->list_all_groups(group_info);

	return result;  
}

int list_one_group( const char* group_name, BufferInfo *group_info)
{
	if (group_name == NULL)
	{
		return FSC_ERROR_CODE_PARAM_INVAILD;
	}

	if(g_pClient == NULL) return FSC_ERROR_CODE_INIT_FAILED;

	int result = 0;
	result= g_pClient->list_one_group(group_name, group_info);

	return result;  
}

int list_storages(const char* group_name, 
				  const char* storage_id, 
				  BufferInfo* storages_info)
{
	if (group_name == NULL)
	{
		return FSC_ERROR_CODE_PARAM_INVAILD;
	}

	if (strcmp(storage_id, "") == 0)
	{
		storage_id = NULL;
	}

	if(g_pClient == NULL) return FSC_ERROR_CODE_INIT_FAILED;

	int result = 0;
	result= g_pClient->list_storages(group_name, storage_id, storages_info);

	return result;  
}
