1、复制文件夹

复制ClientForPython到FastDFS源代码client目录下


2、复制json库
cp json/lib/* /usr/local/lib/
ldconfig


3、接入client目录下 执行Make

(1)python2
make  -f  Makefile

(2)python3
make -f Makefile3

4、进入ClientForPython，生成.so文件

详细说明文档请看：
http://blog.csdn.net/lenyusun/article/details/44057139

==========================================================

2018.3.18 ver 1.2.0
增加了对python3.6的支持

2015.4.13 ver 1.1.0
增加了slave_upload接口


2015.2.28 Ver1.0.0 
初始版本
