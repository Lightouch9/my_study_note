#include"utils.h"
//工具类初始化
void Utils::init()
{

}
//将文件描述符设置为非阻塞
int Utils::setnonblocking(int fd)
{
    //获取文件描述符旧的状态标志
    int old_option=fcntl(fd);
    int new_option=old_option | O_NONBLOCK; //设置非阻塞标志
    fcntl(fd, F_SETFL, new_option); //设置新的文件描述符的属性
    return old_option;  //返回修改前的文件属性
}
//向epoll内核事件表中添加读事件
void Utils::addfd(int epollfd, int fd, bool is_one_shot, int trig_mode)
{
    epoll_event event;
    event.data.fd=fd;
    //触发模式判断
    if(trig_mode==1)
    {
        //ET
        event.events=EPOLLIN|EPOLLET|EPOLLRDHUP;
    }
    else
    {
        //LT
        event.events=EPOLLIN|EPOLLRDHUP;
    }
    //是否注册epolloneshot事件
    if(is_one_shot)
    {
        event.events |= EPOLLONESHOT;
    }
    //添加到epoll内核事件表
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    //设置非阻塞
    setnonblocking(fd);
}
//信号处理函数
void Utils::sig_handler(int sig)
{
    
    int save_errno=errno;   //保证可重入性，保留全局变量errno
    int msg=sig;
    send(m_pipefd[1], (char*)&msg, 1, 0); //发送到管道的写端，统一事件源
    errno=save_errno;
}
//设置信号处理函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;    //用于存储对信号的处理方式
    memset(&sa, '/0', sizeof(sa));  //将此内存重置为0
    sa.sa_handler=handler;  //指定信号处理函数

    if(restart)
        sa.sa_flags |= SA_RESTART;  //如果restart为true，则设置中断重启行为

    sigfillset(&sa.sa_mask);    //设置信号掩码，在处理信号sig时，屏蔽所有信号
    assert(sigaction(sig, &sa, NULL)!=-1);
}