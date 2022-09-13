#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFF_SIZE 1024

int main(void)
{
	int server_sockfd;
	int client_sockfd;
	char ch;
	int ret;
	int recv_len;
	char buff[BUFF_SIZE];

	 //用于UNIX系统内部通信的地址， struct sockaddr_un
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int client_addr_len =sizeof(struct sockaddr_in);
	
	server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// 设置服务器地址
	server_addr.sin_family = AF_INET;  //地址的域，相当于地址的类型, AF_UNIX表示地址位于UNIX系统内部
	server_addr.sin_addr.s_addr = INADDR_ANY;  //inet_addr("10.10.0.9");
	server_addr.sin_port = htons(9000);

	// 绑定该套接字，使得该套接字和对应的系统套接字文件关联起来。
	ret = bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret == -1) {
		perror("bind");
		exit(1);
	}

	// 创建套接字队列， 保存进入该服务器的客户端请求。
	//ret = listen(server_sockfd, 5);

	// 循环处理客户端请求
	while (1) {

		printf("server waiting\n");
		
		// 等待并接收客户端请求
		//client_sockfd = accept(server_sockfd,  (struct sockaddr*)&client_addr, &client_addr_len);
              recv_len = recvfrom(server_sockfd, buff, sizeof(buff) , 0, 
                                           (struct sockaddr*)&client_addr, &client_addr_len);
		if (recv_len < 0) {
			perror("recvfrom");
			exit(errno);
		}

		printf("received: %s\n", buff);	
	}

	close(server_sockfd);

	return 0;	
}
