#include<iostream>
#include"webserver.h"
using namespace std;

int main()
{
    // - 初始化`WebServer`对象`server`
    WebServer server;
    server.init();
    // - 启动`server`的日志记录
    // - 启动`server`的数据库
    // - 启动`server`的线程池
    server.thread_pool();
    // - 设置`server`的触发模式
    server.trig_mode();
    // - 初始化`server`的事件监听设置
    server.eventListen();
    // - 启动`server`的核心事件循环
    server.eventLoop();
    return 0;
}