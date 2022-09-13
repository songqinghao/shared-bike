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
	int sockfd;
	struct sockaddr_in server_addr;
	int ret;
	int c;
	char buff[BUFF_SIZE];
	socklen_t addr_len;

	// 创建一个套接字
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// 设置服务器地址
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(9000);
	
       // 向服务器发送数据
       strcpy(buff, "hello world");
       ret = sendto(sockfd, buff, strlen(buff) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret == -1) {
		perror("sendto");
		exit(errno);
	}

	printf("send %d bytes\n", ret);

	ret = recv(sockfd, buff, sizeof(buff), 0);
	if (ret == -1) {
		perror("recvfrom");
		exit(errno);
	}

	printf("received %d bytes\n", ret);
	printf("received: %s\n", buff);	
	
	return 0;	
}
