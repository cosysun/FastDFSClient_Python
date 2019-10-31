# Interfaces Documentation

[TOC]

## 01. `init(str, int, int, bool)`

|        | `init`            |                |                                |
| ------ | ----------------- | -------------- | ------------------------------ |
| IN     | `config`          | `str`          | FastDFS Client 配置文件路径    |
|        | *`log_level`*     | *`int=-1`*     | *UNIX 日志等级*                |
|        | *`log_fd`*        | *`int=-1`*     | *日志文件描述符*               |
|        | *`take_over_std`* | *`bool=False`* | *重定向`stdout`与`stderr`输出* |
| RETURN | response          | `int`          | `0`为成功，否则失败            |

导入`FDFSPythonClient`模块后，必须首先调用该接口进行初始化操作。其中`config`参数为必须，其余皆为可选。

```python
import FDFSPythonClient as fdfs

ret = fdfs.init('/etc/fdfs/client.conf')
```

`config`文件中定义了`base_path`与`log_level`作为日志存储路径与日志等级，默认情况下会将日志存放于`base_path/logs/cliented.log`。

如若`base_path`未定义或没有读写权限，则会将日志存放于`/tmp/logs/cliented.log`下；若`log_level`未定义，其默认为`LOG_INFO`。日志等级说明如下：

```c++
// syslog.h
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */
```

该接口也支持关键字传参：

```python
import FDFSPythonClient as fdfs

ret = fdfs.init('/etc/fdfs/client.conf', take_over_std=True)
```

`take_over_std`会将`stdout`与`stderr`重定向输出到日志文件中。

如若想将日志输出到其他地方，可指定`log_fd`：

```python
import FDFSPythonClient as fdfs
#import sys
import os

PATH = 'fifo_file'

try:
    os.mkfifo(PATH)
except FileExistsError:
    pass

pipe_r = os.open(PATH, os.O_RDONLY | os.O_NONBLOCK)
pipe_w = os.open(PATH, os.O_WRONLY | os.O_NONBLOCK)

r = fdfs.init('/etc/fdfs/client.conf', log_fd=pipe_w)
f = open(pipe_r, 'rt')
#os.dup2(pipe_w, sys.stderr.fileno())

r = fdfs.download_file('group1', 'M00/00/00/abcdefghijklmnopqrstuvwxyz1234.jpg')
if r[0] != 0:
    logs = f.readlines()
    print(logs)

r = fdfs.destroy()	# pipe_w is closed now.
f.close()	# pipe_r is closed also.
```

由于所有的接口都是有返回值的，所以建议对每个接口都进行赋值操作，避免不必要的标准输出。

## 02. `destroy()`

|        | `destory` |       |                     |
| ------ | --------- | ----- | ------------------- |
| RETURN | response  | `int` | `0`为成功，否则失败 |

销毁 FastDFS 客户端。

为避免内存泄漏，请务必在结束时显式调用该接口。

## 03. `upload_file(bytes, str)`

|        | `upload_file`                |              |                      |
| ------ | ---------------------------- | ------------ | -------------------- |
| IN     | file_buff                    | `bytes`      | 上传文件的二进制内容 |
|        | file_ext_name                | `str`        | 上传文件的扩展名     |
| RETURN | (response, remote_file_name) | `(int, str)` | `0`为成功，否则失败  |

上传**普通类型**的文件，返回一个元组对象：`(0, 'group1/M00/00/00/ABCDEFGHIJKLMNOPQRSTUVWXYZ5678.png')`。

## 04. `upload_slave(bytes, str, str, str)`

|        | `upload_slave`               |              |                     |
| ------ | ---------------------------- | ------------ | ------------------- |
| IN     | file_buff                    | `bytes`      | 从文件的二进制内容  |
|        | master_file_id               | `str`        | 主文件的文件名      |
|        | prefix_name                  | `str`        | 从文件的前缀名      |
|        | file_ext_name                | `str`        | 从文件的扩展名      |
| RETURN | (response, remote_file_name) | `(int, str)` | `0`为成功，否则失败 |

上传主从文件中的**从文件**。

一般来说，前缀名以下划线`_`开头。

`master_file_id`形如：`group1/M00/00/00/ABCDEFGHIJKLMNOPQRSTUVWXYZ5678.png`

## 05. `upload_appender(bytes, str)`

|        | `upload_appender`            |              |                      |
| ------ | ---------------------------- | ------------ | -------------------- |
| IN     | file_buff                    | `bytes`      | 上传文件的二进制内容 |
|        | file_ext_name                | `str`        | 上传文件的扩展名     |
| RETURN | (response, remote_file_name) | `(int, str)` | `0`为成功，否则失败  |

上传**追加类型**的文件，返回一个元组对象：`(0, 'group1/M00/00/00/ABCDEFGHIJKLMNOPQRSTUVWXYZ7890.png')`。

除接口名不同外，其他皆与`upload_file`一样。

## 06. `append_file(bytes, str)`

|        | `upload_appender` |         |                                  |
| ------ | ----------------- | ------- | -------------------------------- |
| IN     | file_buff         | `bytes` | 上传文件的二进制内容             |
|        | appender_file_id  | `str`   | 要追加上传的追加类型文件的文件ID |
| RETURN | response          | `int`   | `0`为成功，否则失败              |

向**追加类型**的文件追加上传文件。

注意：`appender_file_id`形如`group1/M00/00/00/abcdefghijklmnopqrstuvwxyz1234.png`。

## 07. `download_file(str, str)`

|        | `download_file`       |                |                     |
| ------ | --------------------- | -------------- | ------------------- |
| IN     | group_name            | `str`          | 下载文件的存储组名  |
|        | remote_filename       | `str`          | 下载文件的存储名    |
| RETURN | (response, file_buff) | `(int, bytes)` | `0`为成功，否则失败 |

下载文件。

注意：`remote_filename`形如`M00/00/00/abcdefghijklmnopqrstuvwxyz1234.jpg`。

## 08. `delete_file(str, str)`

|        | `delete_file`   |       |                     |
| ------ | --------------- | ----- | ------------------- |
| IN     | group_name      | `str` | 删除文件的存储组名  |
|        | remote_filename | `str` | 删除文件的存储名    |
| RETURN | response        | `int` | `0`为成功，否则失败 |

删除文件。

注意：`remote_filename`形如`M00/00/00/abcdefghijklmnopqrstuvwxyz5678.jpg`。

## 09. `get_file_info(str)`

|        | `get_file_info`       |              |                     |
| ------ | --------------------- | ------------ | ------------------- |
| IN     | file_id               | `str`        | 文件名              |
| RETURN | (response, file_info) | `(int, str)` | `0`为成功，否则失败 |

获取文件信息。

注意：

1. `file_id`形如`group1/M00/00/00/abcdefghijklmnopqrstuvwxyz5678.jpg`。
2. 对于`appender`类型文件来说，其获取的`crc32`值不准确。

## 10. `list_groups()`

|        | `list_groups`           |              |                     |
| ------ | ----------------------- | ------------ | ------------------- |
| RETURN | (response, groups_info) | `(int, str)` | `0`为成功，否则失败 |

list all groups.

## 11. `list_one_group(str)`

|        | `list_one_group`       |              |                     |
| ------ | ---------------------- | ------------ | ------------------- |
| IN     | group_name             | `str`        | the group name      |
| RETURN | (response, group_info) | `(int, str)` | `0`为成功，否则失败 |

list one group.

注意：`group_name`形如`group1`。

## 12. `list_servers(str, str)`

|        | `list_servers`            |              |                                       |
| ------ | ------------------------- | ------------ | ------------------------------------- |
| IN     | group_name                | `str`        | group name to query                   |
|        | storage_id                | `str`        | the storage id to query, can be empty |
| RETURN | (response, storage_infos) | `(int, str)` | `0`为成功，否则失败                   |

list all servers of the specified group.

注意：`group_name`形如`group1`，`storage_id`可以为空字符串。`storage_id`形式定义于`storage_ids.conf`文件，在`tracker.conf`的`use_storage_id`参数设置是否启用，默认为`ip`形式。

---

*注意：只有`init`接口支持关键字参数*

