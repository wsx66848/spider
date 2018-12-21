README

===================

该工程为C语言编写的，使用libevent库的多线程爬虫，以及用C++编写的pagerank算法

****
|Autho|wsx|


****

##libevent

libevent库为2.18-stable版本，需要使用本地的gcc工具编译

##编译

windows可在QT上直接运行，不需要makefile,项目文件为spider.pro
linux上需要安装libsafec,以及修改makefile，修改INLCUDE,LIB路径,注意运行时需设置LD_LIBRARY_PATH来指定动态库的路径

##已知BUG

linux上内存越界

##fix

