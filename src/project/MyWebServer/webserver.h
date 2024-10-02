//头文件引用重复性检查
#ifndef WEBSERVER_H
#define WEBSERVER_H

class WebServer
{
    public:
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
};
#endif