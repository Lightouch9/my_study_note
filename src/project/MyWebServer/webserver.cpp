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
    int ret=0;  //接收返回值的变量
    struct sockaddr_in address;
    bzero(&address, sizeof(address));   //将address内清空，确保这块内存是干净的
    address.sin_family=AF_INET; //设置地址结构的协议族
    address.sin_addr.s_addr=htonl(INADDR_ANY);  //设置绑定本地任意ip
    address.sin_port=htons(m_port); //绑定端口
    //设置地址重用选项
    int reuse=1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    //绑定地址
    ret=bind(m_listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret>=0);
    //设置监听
    ret=listen(m_listenfd, 5);
    assert(ret>=0);
    //初始化工具类
    utils.init();

    //创建epoll内核事件表
    epoll_event events[MAX_EVENT_NUMBER];   //创建存储就绪事件的数组
    m_epollfd=epoll_create(5);  //创建epoll文件描述符
    assert(m_epollfd!=-1);
    //将监听套接字添加进epoll监听的内核事件表中
    utils.addfd(m_epollfd, m_listenfd, false, m_listen_trig_mode);
    //创建双向管道用于信号处理
    ret=socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret>=0);
    utils.setnonblocking(m_pipefd[1]);  //将写端设置为非阻塞
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);  //添加到epoll监听可读事件
    utils.addsig(SIGPIPR, SIG_IGN); //忽略SIGPIPE信号
    

}
//服务器事件循环的开始
void WebServer::eventLoop(){}