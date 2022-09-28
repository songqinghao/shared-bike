#include <sys/types.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define MAX_FD 8192
struct pollfd fds[MAX_FD];
int cur_max_fd = 1;

void setMaxFD(int fd)
{
    if(fd >= cur_max_fd)
    {
        cur_max_fd = fd + 1;
    }

}


int main()
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    //fd_set readfds, testfds;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);//建立服务器端socket 
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9000);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr*)&server_address, server_len);
    listen(server_sockfd, 5); //监听队列最多容纳5个 
    //FD_ZERO(&readfds);
    //FD_SET(server_sockfd, &readfds);//将服务器端socket加入到集合中
    fds[server_sockfd].fd=server_sockfd;
    fds[server_sockfd].events=POLLIN;
    fds[server_sockfd].revents = 0;
    setMaxFD(server_sockfd);
    while (1)
    {
        char ch;
        int i,fd;
        int nread;
        //printf("server waiting\n");

        /*有事件发生  返回revents域不为0的文件描述符个数
         *超时：return 0
         *失败：return  -1   错误：errno 
	 */
        result = poll(fds, cur_max_fd, 2000);
        if (result < 0)
        {
            perror("server5");
            exit(1);
        }
	if (result == 0)
	{
	    printf("timeout!!\n");
	}

        /*扫描所有的文件描述符*/
        for (i = 0; i < cur_max_fd; i++)
        {
            /*找到相关文件描述符 如果是该fd发生事件则revent返回事件类型*/
            if (fds[i].revents)
            {
                fd = fds[i].fd;
                /*判断是否为服务器套接字，是则表示为客户请求连接。*/
                if (fd == server_sockfd)
                {
                    client_len = sizeof(client_address);
                    //记录客户端的fd
                    client_sockfd = accept(server_sockfd,
                        (struct sockaddr*)&client_address, &client_len);
                    //将客户端socket加入到集合中
                    fds[client_sockfd].fd=client_sockfd;
                    fds[client_sockfd].events=POLLIN;
                    fds[client_sockfd].revents = 0;
                    setMaxFD(client_sockfd);
                    printf("adding client on fd %d\n", client_sockfd);
                }
                else
                {
		    /*如果是触发客户端fd的读事件，即有"人"对client_fd进行write或者是close操作*/
                    if(fds[i].revents & POLLIN){
                        nread = read(fd, &ch, 1);//读一个字节，返回数据量
                        /*客户数据请求完毕，关闭套接字，从集合中清除相应描述符 如果数据量为0表示已经请求完毕*/
                        if (nread == 0)
                        {
                            close(fd);//关闭套接字
                            //去掉关闭的fd，其实将event直接设置为0也可以
                            memset(&fds[i], 0, sizeof(struct pollfd));
                            
                            printf("removing client on fd %d\n", fd);
                        }
                        /*处理客户数据请求*/
                        else
                        {
                            sleep(5);
                            printf("serving client on fd %d，receive:%c\n", fd,ch);
                            ch++;
                            fds[i].events = POLLOUT;
                        }
                    }else if(fds[i].revents & POLLOUT){//数据可写，写通道准备好了
                        write(fd, &ch, 1);
                        fds[i].events = POLLIN;//继续监听读数据
                    }
                }
            }
        }
    }

    return 0;
}
