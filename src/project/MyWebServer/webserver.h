//头文件引用重复性检查
#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include"utils.h"
#include"http_conn.h"
#include"threadpool.h"
//全局变量
const int MAX_EVENT_NUMBER = 10000; //最大就绪事件数量
const int TIMESLOT = 5; //最小超时单位
const int MAX_FD = 65535;   //最大文件描述符
class WebServer
{
    public:
        //成员方法

        //构造函数与析构函数
        WebServer();
        ~WebServer();
        //初始化函数
        void init(int port, int trigmode, int thread_num, int actor_mode, int opt_linger);

        //设置监听与连接套接字的触发模式
        void trig_mode();
        //创建线程池
        void thread_pool();
        //事件监听初始化
        void eventListen();
        //服务器事件循环开始
        void eventLoop();
        //处理新的客户端连接请求
        bool deal_client_conn();
        //处理客户端关闭连接，移除该连接的定时器
        void deal_timer(util_timer* timer, int sockfd);
        //处理信号
        bool deal_signal(bool &timeout, bool &stop_server);
        //处理客户端连接上的可读事件
        void deal_read(int sockfd);
        //处理客户端连接上的可写事件
        void deal_write(int sockfd);
        //为客户端的连接初始化定时器
        void timer(int connfd, struct sockaddr_in client_addr);
        //重新调整定时器的过期时间
        void adjust_timer(util_timer* timer);
    
    public:
        //成员变量
        int m_listenfd;   //监听套接字
        int m_port; //监听套接字的端口
        char* m_root;   //服务器根目录
        int m_epollfd;  //epoll文件描述符
        epoll_event events[MAX_EVENT_NUMBER];   //接受就绪事件的数组
        int m_trig_mode;  //总事件触发模式
        int m_listen_trig_mode; //监听文件描述符的事件触发模式，1为边缘触发ET，0为水平触发LT
        int m_conn_trig_mode;   //连接套接字的事件触发模式，1为边缘触发ET，0为水平触发LT
        int m_pipefd[2];  //管道
        
        int m_opt_linger;   //是否延迟关闭连接
        int m_actormodel;   //I/O事件处理模式，1为reactor，0为proactor
        Utils utils;    //工具类
        http_conn* users;   //客户端连接类数组
        client_data* users_data;    //用户数据集合，存储各个用户的地址、套接字、定时器等信息

        threadpool<http_conn>* m_thread_pool;    //处理任务的线程池
        int m_thread_num;   //线程池中线程数量

};
#endif