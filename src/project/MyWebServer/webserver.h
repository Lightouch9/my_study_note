//头文件引用重复性检查
#ifndef WEBSERVER_H
#define WEBSERVER_H
#include<assert.h>
#include<netinet/in.h>
#include"utils.h"
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
    
    public:
        //成员变量
        int m_listenfd;   //监听套接字
        int m_port; //监听套接字的端口
        int m_epollfd;  //epoll文件描述符
        int m_trig_mode;  //事件触发模式
        int m_listen_trig_mode; //监听文件描述符的事件触发模式
        int m_pipefd[2];  //管道
        Utils utils;    //工具类
};
#endif