//webserver.h中各方法的实现
#include "webserver.h"

//构造函数
WebServer::WebServer()
{
    //初始化客户端连接数组
    users=new http_conn[MAX_FD];
    //服务器根目录路径初始化
    char server_path[200];
    getcwd(server_path, 200);   //获取当前路径
    char root[6]="/root/";  //构建根目录路径
    m_root=(char*)malloc(strlen(server_path)+strlen(root)+1);   //动态分配空间
    //设置根目录
    strcpy(m_root, server_path);
    strcat(m_root, root);
    //用户数据集合初始化
    users_data=new client_data[MAX_FD];
}
//析构函数
WebServer::~WebServer()
{
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[0]);
    close(m_pipefd[1]);
    delete[] users;
    delete[] users_data;
    delete[] m_thread_pool;
}
//初始化函数
void WebServer::init(int port, int trigmode, int thread_num, int actor_mode, int opt_linger)
{
    m_port=port;
    m_trig_mode=trigmode;
    m_thread_num=thread_num;
    m_actormodel=actor_mode;
    m_opt_linger=opt_linger;
}
//设置监听与连接套接字的触发模式
void WebServer::trig_mode()
{
    //LT+LT
    if(m_trig_mode==0)
    {
        m_listen_trig_mode=0;
        m_conn_trig_mode=0;
    }
    //LT+ET
    else if(m_trig_mode==1)
    {
        m_listen_trig_mode=0;
        m_conn_trig_mode=1;
    }
    //ET+LT
    else if(m_trig_mode==2)
    {
        m_listen_trig_mode=1;
        m_conn_trig_mode=0;
    }
    //ET+ET
    else if(m_trig_mode==3)
    {
        m_listen_trig_mode=1;
        m_conn_trig_mode=1;
    }
}
//线程池的创建与初始化
void WebServer::thread_pool()
{
    m_thread_pool=new thread_pool<http_conn>(m_actormodel, m_thread_num);
}
//为客户端的连接初始化定时器
void WebServer::timer(int connfd, struct sockaddr_in client_addr)
{
    users[connfd].init(connfd, client_addr, m_root, m_trig_mode);   //初始化客户端连接类
    //初始化新用户连接的client_data数据
    //初始化地址信息、套接字信息
    users_data[connfd].address=client_addr;
    users_data[connfd].sockfd=connfd;
    //初始化定时器
    util_timer* timer=new util_timer;
    timer->user_data=&users_data[connfd];   //初始化定时器的用户数据
    timer->cb_func=cb_func; //初始化回调函数
    timer_t cur=time(NULL);  //获取当前时间
    timer->expire=cur+3*TIMESLOT;   //设置超时时间
    users_data[connfd].timer=timer; //定时器赋值
    utils.m_timer_lst.add_timer(timer);
}
//
void WebServer::adjust_timer(util_timer* timer)
{
    //获取当前时间
    time_t cur=time(NULL);
    //重置定时器的过期时间，往后延迟3个时间单位
    timer->expire=cur + 3 * TIMESLOT;
    //同步调整定时器链表中的定时器
    utils.m_timer_lst.adjust_timer(timer);

}
//处理新的客户端连接请求
bool WebServer::deal_client_conn()
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
        if(http_conn::m_user_count>=MAX_FD)
        {
            //报错
            utils.show_error(connfd, "Internal Server Busy,系统繁忙请稍后再试。");

            return false;
        }
        //为新连接初始化定时器
        timer(connfd, client_addr);
    }
    //ET模式
    else
    {
        while(1)
        {
            //模拟ET，使用循环接受所有可用连接，直到accept返回错误
            //接受连接
            int connfd=accept(m_listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
            if(connfd<0)    //错误检查
            {
                break;
            }
            //检查当前活动的用户数量是否超出限制
            if(http_conn::m_user_count>=MAX_FD)
            {
                //报错
                utils.show_error(connfd, "Internal Server Busy,系统繁忙请稍后再试。");

                break;
            }
            //为新连接初始化定时器
            timer(connfd, client_addr);
        }
        return false;
    }
    return true;
}
//处理客户端关闭连接，移除该连接的定时器
void WebServer::deal_timer(util_timer* timer, int sockfd)
{
    //调用定时器的回调函数
    timer->cb_func(users_data[sockfd]);
    //timer非空检查
    if(timer)
    {
        //移除定时器
        utils.m_timer_lst.del_timer(timer);
    }
}
//处理信号
bool WebServer::deal_signal(bool &timeout, bool &stop_server)
{
    int ret=0;  //接收返回值的变量
    char signals[1024]; //存储接受到的信号
    ret=recv(m_pipefd[0], signals, sizeof(signals), 0); //从管道中接受信号，返回值为接受到的数据的字节数
    //错误检查
    if(ret<=0)
        return false;
    else
    {
        for(int i=0;i<ret;i++)
        {
            switch(signals[i])
            {
                //超时信号
                case SIGALRM:
                {
                    timeout=true;
                    break;
                }
                //终止信号
                case SIGTERM:
                {
                    stop_server=true;
                    break;
                }
            }
        }
    }
    return true;
}
//处理客户端连接上的可读事件
void WebServer::deal_read(int sockfd)
{
    //获取该客户端连接的定时器
    util_timer* timer=users_data[sockfd].timer;
    //reactor模式，服务器监听分发事件，将任务请求放入任务队列，逐个处理任务
    if(m_actormodel==1)
    {
        //重新调整定时器
        if(timer)
        {
            adjust_timer(timer);
        }
        //将该可读任务(state=0)添加进任务队列
        m_thread_pool->append(users+sockfd, 0);
        //处理改进标志
        while(true)
        {
            if(users[sockfd].improv==1)
            {
                if(users[sockfd].timer_flag==1)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag=0;
                }
                users[sockfd].improv=0;
                break;
            }
        }
    }
    //proactor模式，此处是通过同步I/O模拟的proactor，由主线程监听和读取数据并构造任务加入任务队列
    else
    {
        //尝试读取数据
        if(users[sockfd].read_once())
        {
            //将事件放入任务队列
            m_thread_pool->append_p(users+sockfd);
            //重新调整定时器
            if(timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}
//处理客户端连接上的可读事件
void WebServer::deal_write(int sockfd)
{
    //获取定时器
    util_timer* timer=users_data[sockfd].timer;
    //reactor
    if(m_actormodel==1)
    {
        //更新定时器
        if(timer)
            adjust_timer(timer);
        //将写任务(state=1)添加进任务队列
        m_thread_pool->append(users+sockfd, 1);
        //处理改进标志
        while(true)
        {
            //等待工作子线程处理该读取任务
            if(users[sockfd].improv==1)
            {
                //工作子线程读取失败
                if(users[sockfd].timer_flag==1)
                {
                    deal_timer(timer, sockfd);
                    //重置定时器标志
                    users[sockfd].timer_flag=0;
                }
                users[sockfd].improv=0;
                break;
            }
        }
    }
    //proactor，同步模拟
    else
    {
        if(users[sockfd].write())
        {
            //重新更新定时器
            if(timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}
//事件监听的创建与初始化
void WebServer::eventListen()
{
    //创建监听用的网络套接字
    m_listenfd=socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd>=0);
    //优雅关闭连接
    if(m_opt_linger==0)
    {
        //当连接关闭仍有未发送的数据时不延迟关闭连接
        struct linger tmp={0,1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if(m_opt_linger==1)
    {
        struct linger tmp={1,1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

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
    utils.addsig(SIGALRM, utils.sig_handler, false);    //设置定时器到期信号处理函数
    utils.addsig(SIGTERM, utils.sig_handler, false);    //设置终止信号处理函数

    //向工具类传入管道和epoll文件描述符方便使用
    utils.m_pipefd=m_pipefd;
    utils.m_epollfd=m_epollfd;
    
    alarm(TIMESLOT);    //设置定时器，定时处理其他到期的定时器

}
//服务器事件循环的开始
void WebServer::eventLoop()
{
    //初始化超时标志与服务器停止标志，局部变量
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
                bool flag=deal_client_conn();
                //如果新连接建立失败
                if(!flag)
                {
                    continue;
                }
            }
            //发生错误或客户端关闭了连接
            else if(events[i].events&(EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，移除定时器
                util_timer* timer=users_data[sockfd].timer; //获取该连接的定时器
                deal_timer(timer, sockfd);  //移除该连接的定时器
            }
            //处理信号，管道有可读事件
            else if((sockfd==m_pipefd[0])&&(events[i].events & EPOLLIN))
            {
                bool flag=deal_signal(timeout, stop_server);
                if(flag==false)
                    printf("deal_signal error\n");
            }
            //处理客户端连接的上接受到的数据
            //可读事件
            else if(events[i].events & EPOLLIN)
            {
                deal_read(sockfd);
            }
            //可写事件
            else if(events[i].events & EPOLLOUT)
            {
                deal_write(sockfd);
            }
        }
        //检测超时标志
        if(timeout)
        {
            utils.timer_handler();
            //超时标志重置
            timeout=false;
        }
    }
    
}