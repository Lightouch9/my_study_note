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
//为客户端的连接初始化定时器
void WebServer::timer(int connfd, struct sockaddr_in client_addr)
{
    users.init(connfd, client_addr, )   //初始化客户端连接类
    //初始化client_data数据
}
//处理新的客户端连接请求
bool WebServer::client_conn_requ()
{
    struct sockaddr_in client_addr; //创建存储客户端地址信息的变量
    socklen_t client_addr_len=sizeof(client_addr);  //存储客户端地址结构的大小
    //LT模式
    if(m_listen_trig_mode==0)
    {
        //接受连接
        int connfd=accept(m_listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(connfd<0)    //错误检查
        {
            return false;
        }
        //检查当前活动的用户数量是否超出限制
        if(){}
        //为新连接初始化定时器

    }
    //ET模式
}
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
    //向工具类传入管道和epoll文件描述符方便使用
    utils.m_pipefd=m_pipefd;
    utils.m_epollfd=m_epollfd;

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
    utils.addsig(SIGALRM, utils.sig_handler, false);    //设置定时器到期信号处理函数
    utils.addsig(SIGTERM, utils.sig_handler, false);    //设置终止信号处理函数
    
    alarm(TIMESLOT);    //设置定时器，定时处理其他到期的定时器

}
//服务器事件循环的开始
void WebServer::eventLoop()
{
    //初始化超时标志与服务器停止标志    局部变量
    bool timeout=false;
    bool stop_server=false;
    //核心事件监听循环
    while (!stop_server)
    {
        int num=epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);    //阻塞式监听
        if(num<0 && errno!=EINTR)   //错误检查
        {
            break;
        }
        //遍历就绪事件数组中的事件，对不同事件做出对应的处理
        for(int i=0;i<number;i++)
        {
            int sockfd=events[i].data.fd;   //获取就绪事件的文件描述符
            //根据事件类型进行处理
            //新的客户连接，来自监听socket的是新的客户端连接请求
            if(sockfd==m_listenfd)
            {

            }
        }
    }
    
}