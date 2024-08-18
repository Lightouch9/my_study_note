#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
int main()
{
    //创建监听用套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    //如果创建失败
    if(lfd==-1)
    {
        perror("socket failed");
        exit(0);
    }

    //将套接字与本地ip端口绑定
    //创建套接字地址数据结构
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;    //ipv4
    addr.sin_port=htons(10000); //端口转换为网络字节序
    addr.sin_addr.s_addr=INADDR_ANY;    //本地任意ip
    int ret=bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret==-1)
    {
        perror("bind failed");
        exit(0);
    }
    //设置监听
    ret=listen(lfd, 128);
    if(ret==-1)
    {
        perror("listen failed");
        exit(0);
    }
    //等待客户端连接
    struct sockaddr_in cli_addr;
    int cli_len=sizeof(cli_addr);
    int cfd=accept(lfd, (struct sockaddr*)&cli_addr, &cli_len);
    if(cfd==-1)
    {
        perror("accept failed");
        exit(0);
    }
    //输出客户端信息
    char cli_ip[24]={0};
    printf("客户端ip:%s,端口:%d\n",
            inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, cli_ip, sizeof(cli_ip)),
            ntohs(cli_addr.sin_port));
    //与客户端通信
    while(1)
    {
        //接收数据
        //创建缓冲区
        char buf[1024];
        //填充缓冲区为0
        memset(buf, 0, sizeof(buf));
        //读取
        int len=read(cfd, buf, sizeof(buf));
        if(len>0)
        {
            //接收到了数据
            printf("从客户端接受到的数据:%s\n",buf);
            write(cfd, buf, len);
        }
        else if(len==0)
        {
            printf("客户端连接断开。。。\n");
            break;
        }
        else
        {
            perror("read failed");
            break;
        }
    }
    //释放文件描述符
    close(lfd);
    close(cfd);
    return 0;
}