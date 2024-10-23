#include"utils.h"
//工具类初始化
void Utils::init(int timeslot)
{
    m_TIMESLOT=timeslot;
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
//发送错误信息并关闭连接
void Utils::show_error(int connfd, const char* info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}
//定时处理，重新定时以不断触发SIGALRM信号
void Utils::timer_handler()
{
    //每次触发SIGALARM信号都将调用tick()处理定时器链表上到期的任务
    m_timer_lst.tick();
    //重新定时
    alarm(m_TIMESLOT);
}
//定时器到期的回调函数
void cb_func(client_data* user_data)
{
    //确保user_data不为空
    assert(user_data);
    //清除epoll事件表上的定时器到期的事件
    epoll_ctl(Utils::m_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    //关闭连接
    close(user_data->sockfd);
    //用户数量更新
    http_conn::m_user_count--;
}

//定时器链表析构函数
timer_lst::~timer_lst()
{
    //依次释放删除每个结点
    util_timer* tmp=head;
    while(tmp)
    {
        head=tmp->next;
        delete tmp;
        tmp=head;
    }
}
//定时器链表中添加定时器，按照到期时间expire升序插入
void timer_lst::add_timer(util_timer* timer)
{
    //检查timer是否为空
    if(!timer)
        return;
    //如果链表为空
    if(!head)
    {
        head=tail=timer;
        return;
    }
    //如果添加结点的到期时间比头结点小
    if(timer->expire<head->expire)
    {
        head->prev=timer;
        timer->next=head;
        head=timer;
        return;
    }
    //以指定结点为头结点插入新的结点
    add_timer(timer,head);
}
//允许指定头结点的添加定时器，重载
void timer_lst::add_timer(util_timer* timer, util_timer* lst_head)
{
    //获取头结点和其后继结点
    util_timer* pre=head;
    util_timer* tmp=pre->next;
    //以tmp为基点进行遍历
    while(tmp)
    {
        //如果tmp的expire大于timer,将timer插入到tmp之前
        if(timer->expire<tmp->expire)
        {
            timer->next=tmp;
            timer->prev=pre;
            tmp->prev=timer;
            pre->next=timer;
            break;
        }
        //否则继续向后遍历
        pre=tmp;
        tmp=tmp->next;
    }
    //遍历到链表的尾部
    if(!tmp)
    {
        timer->next=NULL;
        timer->prev=pre;
        pre->next=timer;
        tail=timer;
    }
}
//删除定时器
void timer_lst::del_timer(util_timer* timer)
{
    //非空检查
    if(!timer)
        return;
    //链表中仅此一个结点
    if((timer==head)&&(timer==tail))
    {
        delete timer;
        head==NULL;
        tail==NULL;
        return;
    }
    //删除的是头结点
    if(timer==head)
    {
        head=head->next;
        head->prev=NULL;
        delete timer;
        return;
    }
    //删除的是尾结点
    if(timer==tail)
    {
        tail=tail->prev;
        tail->next=NULL;
        delete timer;
        return;
    }
    //删除链表中间的结点
    timer->prev->next=timer->next;
    timer->next->prev=timer->prev;
    delete timer;
}
//调整链表中的某个定时器
void timer_lst::adjust_timer(util_timer* timer)
{
    //非空检查
    if(!timer)
    {
        return;
    }
    //获取传入定时器的后继
    util_timer* tmp=timer->next;
    if(!tmp || (timer->expire<tmp->expire))
    {
        //如果后继为空(尾结点)，或当前结点过期时间小于后继则无需调整
        return;
    }
    //调整结点为头结点
    if(timer==head)
    {
        //将后继结点设置为头结点
        head=head->next;
        //将要调整的结点断开，并重新添加进链表
        head->prev=NULL;
        timer->next=NULL;
        add_timer(timer, head);
    }
    //不是头结点
    else
    {
        //断开该节点，并重新添加进链表
        timer->prev->next=timer->next;
        timer->next->prev=timer->prev;
        add_timer(timer, timer->next);
    }
}
//处理定时器链表上的任务
 timer_lst::tick()
 {
    //非空检查
    if(!head)
    {
        return;
    }
    //获取当前时间
    time_t cur=time(NULL);
    //获取头结点
    util_timer* tmp=head;
    //从头结点一次遍历，直到遇到未过期的定时器
    while(tmp)
    {
        if(cur<tmp->expire)
        {
            break;
        }
        //调用回调函数，执行定时任务
        tmp->cb_func(tmp->user_data);
        //执行完回调函数后将定时器从链表中删除，更新头结点位置
        head=tmp->next;
        //如果后继结点不为空
        if(head)
        {
            head->prev=NULL;
        }
        delete tmp;
        tmp=head;
    }
 }