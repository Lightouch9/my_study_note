//头文件引用重复性检查
#ifndef WEBSERVER_H
#define WEBSERVER_H
#include<assert.h>
#include<netinet/in.h>
#include"utils.h"
#include"http_conn.h"
//全局变量
const int MAX_EVENT_NUMBER = 10000; //最大就绪事件数量
const int TIMESLOT = 5; //最小超时单位
class WebServer
{
    public:
        //成员方法

        //构造函数与析构函数
        WebServer();
        ~WebServer();
        //初始化函数
        void init();

        //创建线程池
        void thread_pool();
        //事件监听初始化
        void eventListen();
        //服务器事件循环开始
        void eventLoop();
        //处理新的客户端连接请求
        bool client_conn_requ();
        //为客户端的连接初始化定时器
        void timer(int connfd, struct sockaddr_in client_addr);
    
    public:
        //成员变量
        int m_listenfd;   //监听套接字
        int m_port; //监听套接字的端口
        int m_epollfd;  //epoll文件描述符
        epoll_event events[MAX_EVENT_NUMBER];   //接受就绪事件的数组
        int m_trig_mode;  //事件触发模式，1为边缘触发ET，0为水平触发LT
        int m_listen_trig_mode; //监听文件描述符的事件触发模式
        int m_pipefd[2];  //管道
        Utils utils;    //工具类
        http_conn* users;   //客户端连接类数组
};
#endif