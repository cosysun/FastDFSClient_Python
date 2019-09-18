# FastDFS Client for Python

### How to Compile

编译环境：`Ubuntu 18.04`，以下步骤仅供参考：

1. 安装`libfastcommon`与`fastdfs`环境；

2. 安装`python3-dev`依赖（提供`Python.h`文件）；

3. 编译生成`FDFSPythonClient.so`文件。

一种可能的编译步骤：

```bash
sudo apt install git make gcc-4.8 -y
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 40

# install libfastcommon
cd /usr/local/src/
sudo git clone https://github.com/happyfish100/libfastcommon.git --depth 1
cd libfastcommon/
sudo ./make.sh && sudo ./make.sh install

# install fastdfs
cd ../
sudo git clone https://github.com/happyfish100/fastdfs.git --depth 1
cd fastdfs/
sudo ./make.sh && sudo ./make.sh install

# install FastDFS Python Client
sudo apt install python3-dev g++-4.8 -y
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 40

cd ../
sudo git clone https://github.com/cosysun/FastDFSClient_Python.git --depth 1
cd FastDFSClient_Python/python_client
sudo make install
```

详细说明请看：[《FastDFS客户端(Python版)指南》](https://blog.csdn.net/lenyusun/article/details/44057139) 或[《接口说明文档》](./Interfaces.md)。

### Revision History

- 2019.9.18 ver 3.1.0
  - 增加了`get_file_info`接口

- 2019.7.20 ver 3.0.1
  - 修复了拼写错误：`destory`→`destroy`

- 2019.7.19 ver 3.0.0
  - 移除了对`jsoncpp`的依赖
  - 更新了`list_groups`、`list_one_group`和`list_servers`方法。

- 2019.7.3 ver 2.2.2
  - 完善了错误日志输出信息

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
