//头文件
#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>
//宏定义
//用户最大数量，即允许的最大并发连接数
#define USER_LIMIT 5
//读缓冲区大小
#define BUFFER_SIZE 64
//文件描述符数量限制，限制客户端连接数
#define FD_LIMIT 65535
//客户端信息
struct client_data
{
    //客户端socket地址
    struct sockaddr_in address;
    //写缓冲区，要写入客户端的数据
    char* write_buf;
    //读缓冲区，从客户端读入的数据
    char buf[BUFFER_SIZE];
};
//设置非阻塞模式函数
int setnonblocking(int fd)
{
    //获取当前状态标志
    int old_option=fcntl(fd, F_GETFL);
    //设置新的状态标志，添加非阻塞标志
    int new_option=old_option | O_NONBLOCK;
    //将新的状态标志添加进文件描述符
    fcntl(fd, F_SETFL, new_option);
    //返回旧标志集
    return old_option;
};
//主函数
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
    //创建套接字地址的结构体
    struct sockaddr_in address;
    //清空刚创建的address的内存块
    bzero(&address, sizeof(address));
    //设置地址族、ip、端口
    address.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port=htons(port);
    //创建监听套接字
    int listenfd=socket(AF_INET, SOCK_STREAM, 0);
    //断言
    assert(listenfd>=0);
    //绑定套接字到指定的ip地址和端口
    int ret=bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret>=0);
    //设置监听状态
    ret=listen(listenfd, 5);
    assert(ret>=0);

    //创建client_data数组
    //struct client_data* users = new client_data[FD_LIMIT];
    struct client_data* users = (struct client_data*)malloc(FD_LIMIT * sizeof(struct client_data));

    //创建pollfd数组
    struct pollfd fds[USER_LIMIT+1];
    //记录当前连接的客户端数量
    int user_counter=0;
    //初始化fds
    for(int i=1; i<=USER_LIMIT;++i)
    {
        fds[i].fd=-1;
        fds[i].events=0;
    }
    //fds[0]用于监听套接字
    fds[0].fd=listenfd;
    fds[0].events=POLLIN | POLLERR;

    while(1)
    {
        //监听fds中的文件描述符
        ret=poll(fds, user_counter+1, -1);
        if(ret<0)
        {
            printf("poll执行失败。。。\n");
            break;
        }
        for(int i=0;i<user_counter+1;++i)
        {
            //当有新连接请求时
            if((fds[i].fd=listenfd)&&(fds[i].events&POLLIN))
            {
                //接受新连接，创建与客户端通信的套接字
                struct sockaddr_in client_address;
                socklen_t client_addrlength=sizeof(client_address);
                int connfd=accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                if(connfd<0)
                {
                    printf("发生错误：%d",errno);
                    continue;
                }
                //如果连接数量达到限制，则关闭新连接
                if(user_counter>=USER_LIMIT)
                {
                    const char* info="连接达到限制。。。\n";
                    printf("%s", info);
                    //向客户端发送反馈
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                //修改当前连接数量
                user_counter++;
                users[connfd].address=client_address;
                //将新连接设置为非阻塞
                setnonblocking(connfd);
                //将新连接添加进fds进行监听
                fds[user_counter].fd=connfd;
                fds[user_counter].events=POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents=0;
                //输出当前连接情况
                printf("新连接已建立，当前连接数量：%d\n", user_counter);
            }
            //处理错误信息
            else if(fds[i].revents&POLLERR)
            {
                //打印错误信息
                printf("监听的文件描述符%d发生错误\n",fds[i].fd);
                //定义存储错误信息的数组
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length=sizeof(errors);
                //获取错误信息
                if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length)<0)
                {
                    //获取错误信息失败
                    printf("获取socket选项失败。。。\n");
                }
                continue;
            }
            //处理远程客户端关闭连接
            else if(fds[i].revents&POLLRDHUP)
            {
                //将当前关闭的客户端数据用最后一个用户的数据覆盖
                users[fds[i].fd]=users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i]=fds[user_counter];
                i--;
                user_counter--;
                printf("一个客户端关闭了连接。。。\n");
            }
            //处理读事件
            else if(fds[i].revents&POLLIN)
            {
                int connfd=fds[i].fd;
                //清空读缓冲区
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                //接受数据到读缓冲区
                ret=recv(connfd, users[connfd].buf, BUFFER_SIZE-1, 0);
                printf("接受到来自文件描述符%d的%d字节数据:%s。。。\n",connfd, ret, users[connfd].buf);
                //如果读操作发生错误，则关闭连接
                if(ret<0)
                {
                    if(errno!=EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd]=users[fds[user_counter].fd];
                        fds[i]=fds[user_counter];
                        i--;
                        user_counter--;
                    }
                }
                else if(ret==0)
                {
                    //客户端关闭了连接
                }
                else
                {
                    //通知其他客户端准备写数据
                    for(int j=1;j<=user_counter;++j)
                    {
                        if(fds[j].fd==connfd)continue;
                        //清除客户端上的POLLIN事件，暂时不在监视读事件
                        fds[j].events &= ~POLLIN;
                        //添加POLLOUT事件，准备写数据
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf=users[connfd].buf;
                    }
                }
            }
            //处理写事件
            else if(fds[i].revents&POLLOUT)
            {
                int connfd=fds[i].fd;
                //如果写缓冲区为空，跳过
                if(!users[connfd].write_buf)
                {continue;}
                //发送数据
                ret=send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                //清空写缓冲区
                users[connfd].write_buf=NULL;
                //重新注册可读事件
                fds[i].events &= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    //清理回收关闭
    free(users);
    close(listenfd);
    return 0;
}