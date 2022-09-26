client.c 为客户端代码
gcc client.c -o c1.exe -levent

server1.c 单纯使用libevent的服务器端代码
gcc server1.c -o s1.exe -levent

server2.c 为bufferevent的服务器代码，可用client.c做客户端
gcc server2.c -o s2.exe -levent

