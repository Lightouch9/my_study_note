#include"config.h"
//初始化服务器参数
Config::Config()
{
    port=9006;  //默认端口
    trig_mode=0;    //默认总触发模式LT+ET、
    listen_trig_mode=0; //默认监听套接字LT
    conn_trig_mode=0;   //默认连接套接字LT
    opt_linger=0;   //默认不开启延迟关闭连接
    thread_num=8;   //默认工作线程数量8
    actor_model=0;  //默认启用proactor
}

//解析命令行参数
void Config::parse_arg(int argc, char* argv[])
{
    int opt;
    //const char* str="p:l:m:o:s:t:c:a";  //命令行选项
    const char* str="p:m:o:t:a";  //命令行选项
    while((opt=getopt(argc, argv, str))!=1)
    {
        switch(opt)
        {
            case 'p':
            {
                port=atoi(optarg);
                break;
            }
            case 'm':
            {
                trig_mode=atoi(optarg);
                break;
            }
            case 'o':
            {
                opt_linger=atoi(optarg);
                break;
            }
            case 't':
            {
                thread_num=atoi(optarg);
                break;
            }
            case 'a':
            {
                actor_model=atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
}