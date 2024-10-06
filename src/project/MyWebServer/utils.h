#ifndef UTILS_H
#define UTILS_H
//工具类，用于提供一些辅助操作
class Utils
{
    public:
        //构造与析构函数
        Utils(){}
        ~Utils(){}
        //初始化函数
        void init();
        //设置文件描述符为非阻塞
        int setnonblocking(int fd);
        //向epoll内核事件表中添加事件
        void addfd(int epollfd, int fd, bool is_one_shot, int trig_mode);
        //设置信号处理行为
        void addsig(int sig, void(handler)(int), bool restart);
};







#endif