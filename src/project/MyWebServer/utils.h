#ifndef UTILS_H
#define UTILS_H
#include<assert.h>
#include<stdlib.h>
//工具类，用于提供一些辅助操作
class Utils
{
    public:
        //构造与析构函数
        Utils(){}
        ~Utils(){}
        //初始化函数
        void init();
        //设置文件描述符为非阻塞
        int setnonblocking(int fd);
        //向epoll内核事件表中添加事件
        void addfd(int epollfd, int fd, bool is_one_shot, int trig_mode);
        //信号处理函数
        void sig_handler(int sig);
        //设置信号处理函数
        void addsig(int sig, void(handler)(int), bool restart);

    public:
        //成员变量
        static int *m_pipefd;   //存储管道
        static int m_epollfd;   //存储epoll文件描述符

};







#endif