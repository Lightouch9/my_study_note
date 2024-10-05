#include"utils.h"
//向epoll内核事件表中添加事件
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