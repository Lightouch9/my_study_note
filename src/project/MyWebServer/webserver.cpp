//webserver.h中各方法的实现
#include "webserver.h"

//构造函数
WebServer::WebServer(){}
//析构函数
WebServer::~WebServer(){}
//初始化函数
void WebServer::init(){}
//线程池的创建与初始化
void WebServer::thread_pool(){}
//事件监听的创建与初始化
void WebServer::eventListen()
{
    //创建监听用的网络套接字
    m_listenfd=socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd>=0);

    //初始化地址结构
    int ret=0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));   //将address内清空，确保这块内存是干净的
    address.sin_family=AF_INET; //设置地址结构的协议族
    address.sin_addr.s_addr=htonl(INADDR_ANY);  //设置绑定本地任意ip
    address.sin_port=htons(m_port); //绑定端口

}
//服务器事件循环的开始
void WebServer::eventLoop(){}