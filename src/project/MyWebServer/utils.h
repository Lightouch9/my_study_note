#ifndef UTILS_H
#define UTILS_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include"http_conn.h"
//客户数据数据结构
struct client_data
{
    //客户端的地址
    sockaddr_in address;
    //连接套接字
    int sockfd;
    //定时器
    util_timer *timer;
}
//定时器类
class util_timer
{
    public:
        //构造函数
        util_timer():prev(NULL), next(NULL){}
    
    public:
        //成员变量
        time_t expire;  //过期时间
        void (* cb_func)(client_data*); //函数指针，即回调函数，参数为指向client_data的指针
        client_data* user_data;  //用户数据，与该定时器相关联的用户数据，方便回调函数使用
        util_timer* prev;   //定时器链表的前一项
        util_timer* next;   //定时器链表的后一项
}
//定时器链表类
class timer_lst
{
    public:
        timer_lst():head(NULL), tail(NULL){};
        ~timer_lst();

        //添加定时器
        void add_timer(util_timer* timer);
        //删除定时器
        void del_timer(util_timer* timer);
        //调整链表中的某个定时器
        void adjust_timer(util_timer* timer);
        //处理定时器链表上的任务
        void tick();
    private:
        //允许指定头结点的添加定时器，重载
        void add_timer(util_timer* timer, util_timer* lst_head);

        util_timer* head;   //头结点
        util_timer* tail;   //尾结点
}
//工具类，用于提供一些辅助操作
class Utils
{
    public:
        //构造与析构函数
        Utils(){};
        ~Utils(){}
        //初始化函数
        void init(int timeslot);
        //设置文件描述符为非阻塞
        int setnonblocking(int fd);
        //向epoll内核事件表中添加事件
        void addfd(int epollfd, int fd, bool is_one_shot, int trig_mode);
        //信号处理函数
        void sig_handler(int sig);
        //设置信号处理函数
        void addsig(int sig, void(handler)(int), bool restart);
        //发送错误信息并关闭连接
        void show_error(int connfd, const char* info);
        //定时处理，重新定时以不断触发SIGALRM信号
        void timer_handler();

    public:
        //成员变量
        static int *m_pipefd=0;   //存储管道
        static int m_epollfd=0;   //存储epoll文件描述符
        timer_lst m_timer_lst;  //定时器链表
        int m_TIMESLOT; //最小超时时间

};
    //定时器到期的回调函数
    void cb_func(client_data* user_data);

#endif