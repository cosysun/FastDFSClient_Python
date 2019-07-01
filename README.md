# FastDFS Client for Python

### How to Compile

编译环境：`Ubuntu 18.04`，以下步骤仅供参考。

1. 安装`libfastcommon`与`fastdfs`环境

2. 安装`python3-dev`与`libjsoncpp-dev`依赖

   ```bash
   apt install -y python3-dev libjsoncpp-dev g++
   ```

   - 其中，`python3-dev`提供`Python.h`文件，`libjsoncpp-dev`提供`jsoncpp/json/json.h`与`libjsoncpp.so`文件。

   - 另外，由于我们使用的`libjsoncpp.so`是**Ubuntu Developers**维护的版本，故需用匹配的`g++`版本进行编译连接，否则会出现连接符号不一致的情况：

     ```diff
     -000000000002bc9e T _ZNK4Json5Value14toStyledStringEv
     +000000000001a7d0 T _ZNK4Json5Value14toStyledStringB5cxx11Ev
     ```

3. 编译生成`FDFSPythonClient.so`文件

详细说明文档请看：[FastDFS客户端(Python版)指南](https://blog.csdn.net/lenyusun/article/details/44057139) 或 Wiki

### Revision History

- 2019.7.1 ver 2.2.1
  - 兼容了`Python 2.7`

- 2019.6.28 ver 2.2.0
  - 增加了`upload_appender`接口
  - 增加了`append_file`接口
  - 修改了其他接口名，如`slave_upload`接口修改为`upload_slave`

- 2019.6.27 ver 2.1.0
  - 升级了`fdfs_init`方法，支持日志输出和关键字可选传参

- 2019.6.14 ver 2.0.0
  - 移除了对`Python 2`的支持
  - 移除了对`jsoncpp`自行编译的依赖
  - 完善了对`Python 3.6`的编译依赖
- 2018.3.18 ver 1.2.0
  - 增加了对`python3.6`的支持
- 2015.4.13 ver 1.1.0
  - 增加了`slave_upload`接口
- 2015.2.28 ver 1.0.0
  - 初始版本

