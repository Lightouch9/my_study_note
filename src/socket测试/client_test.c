#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
int main()
{
    //创建通信用套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    //如果创建失败
    if(fd==-1)
    {
        perror("socket failed");
        exit(0);
    }

    //连接服务器端
    //创建套接字地址数据结构
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;    //ipv4
    addr.sin_port=htons(10000); //端口转换为网络字节序
    addr.sin_addr.s_addr=INADDR_ANY;    //本地任意ip
    int ret=connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret==-1)
    {
        perror("connect failed");
        exit(0);
    }

    //与服务器端通信
    int number=0;
    while(1)
    {
        //发送数据
        //创建缓冲区
        char buf[1024];
        //填充缓冲区为发送信息
        sprintf(buf, "来自客户端的信息number:%d\n",number);
        //发送数据
        write(fd, buf, strlen(buf)+1);
        //接受服务器端的数据
        //重置缓冲区数据
        memset(buf, 0, sizeof(buf));
        int len=read(fd, buf, sizeof(buf));
        if(len>0)
        {
            //接收到了数据
            printf("从服务器端接受到的数据:%s\n",buf);
        }
        else if(len==0)
        {
            printf("服务器端连接断开。。。\n");
            break;
        }
        else
        {
            perror("read failed");
            break;
        }
        //发送消息间隔
        sleep(1);
    }
    //释放文件描述符
    close(fd);
    return 0;
}