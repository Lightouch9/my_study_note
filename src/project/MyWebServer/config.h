#ifndef CONFIG_H
#define CONFIG_H

#include"webserver.h"

using namespace std;

class Config
{
    public:
        Config();
        ~Config();

        void parse_arg(int argc, char* argv[]);   //解析命令行参数

        //成员变量

        int port;   //端口号
        int trig_mode;  //总触发模式
        int listen_trig_mode;   //监听文件描述符触发模式
        int conn_trig_mode; //连接套接字触发模式
        int opt_linger; //是否延迟关闭
        int thread_num; //线程池工作线程数量
        int actor_model;    //工作模式 reactor/proactor
}

#endif