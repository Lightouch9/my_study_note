//头文件包含
#define _GNU_SOURCE 1   //启用GNU扩展
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<poll.h>
#include<fcntl.h>
#include<libgen.h>

//定义缓冲区大小 64字节
#define BUFFER_SIZE 64
int main(int argc, char* argv[])
{
    //命令行参数检查
    if(argc<=2)
    {
        //检查是否提供必要的参数，服务器ip与端口
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    //设置服务器ip与端口，从命令行读取
    const char* ip=argv[1];
    int port=atoi(argv[2]);
    //创建存储服务器地址的结构体
    struct sockaddr_in server_address;
    //清空刚创建的server_address的内存块
    bzero(&server_address, sizeof(server_address));
    //设置地址族、ip、端口
    server_address.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port=htons(port);
    //创建套接字
    int sockfd=socket(AF_INET, SOCK_STREAM, 0);
    //断言
    assert(sockfd>=0);

    if(connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address))<0)
    {
        //如果连接失败打印错误信息
        printf("连接服务器失败。。。\n");
        close(sockfd);
        return 1;
    }
    //设置pollfd用于poll
    struct pollfd fds[2];
    //注册文件描述符0，监听标准输入的可读事件
    fds[0].fd=0;
    fds[0].events=POLLIN;
    fds[0].revents=0;
    //注册文件描述符1，监听用于网络通信的文件描述符的可读与关闭事件
    fds[1].fd=sockfd;
    fds[1].events=POLLIN | POLLRDHUP;
    fds[1].revents=0;

    //创建管道
    int pipefd[2];
    int ret=pipe(pipefd);
    //设置断言
    assert(ret!=-1);
    //创建读缓冲区
    char read_buf[BUFFER_SIZE];
    //核心循环
    while(1)
    {
        //阻塞监听标准输入与网络套接字
        ret=poll(fds, 2, -1);
        //执行错误，退出循环
        if(ret<0)
        {
            printf("poll监听错误。。。\n");
            break;
        }
        //处理服务器关闭套接字
        if(fds[1].revents & POLLRDHUP)
        {
            printf("服务器端关闭了连接。。。\n");
            break;
        }
        //处理服务器套接字是有数据可读，即服务器发送了数据
        else if(fds[1].revents & POLLIN)
        {
            //将读缓冲区清零，确保缓冲区没有干扰数据
            memset(read_buf, '\0', BUFFER_SIZE);
            //接受数据到缓冲区中
            recv(fds[1].fd, read_buf, BUFFER_SIZE-1, 0);
            //输出接收到的数据
            printf("%s\n", read_buf);
        }
        //处理用户输入事件
        if(fds[0].revents & POLLIN)
        {
            ret=splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            //若传输的数据量小于0，则出现了错误
            if(ret<0)
            {
                perror("splice");
                break;
            }
            //从管道的读端将数据写入sockfd发送给服务器
            ret=splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            //若传输的数据量小于0，则出现了错误
            if(ret<0)
            {
                perror("splice");
                break;
            }
        }
    }
    //关闭套接字
    close(sockfd);
    return 0;
}