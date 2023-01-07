# 🚀shared-bike
这是一个我的共享单车项目，该项目想要完成的最终效果是使用Linux实现高并发服务器以及使用Qt实现相应的用户界面，以此来模拟用户使用共享单车的情形。

Linux高并发服务器部分使用了单例和发布-订阅模式，包装头部来处理TCP分包和粘包情况，使用protocbuf实现应用层协议，libevent处理网络连接，makefile编译项目等等。


高并发服务器的Linux部分已经完成，Qt部分正在进行中...

# 文件夹说明

- src文件夹：该项目的源代码实现


- fork文件夹：进程的一些应用（创建+销毁+多进程高并发框架...）
- multiplexing文件夹：IO多路复用的应用（epoll+select+poll...）
- protoMessage文件夹：序列标准化的应用
- signal文件夹：信号量的应用
- mem_poll文件夹：内存池的设计与应用（手写内存池）
- udpCom文件夹：udp通信的应用
- sharemem文件夹：共享内存的使用（System V版本的共享内存shmm+内存映射mmap）
- threadInfo文件夹：线程的相关操作以及线程池的实现
- test文件夹:用于单元测试