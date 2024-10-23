//头文件引用重复性检查
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include"locker.h"
#include"utils.h"
class http_conn
{
    public:
        static const int FILENAME_LEN = 200;    //文件名长度
        static const int READ_BUFFER_SIZE = 2048;   //读缓冲区大小
        static const int WRITE_BUFFER_SIZE = 1024;  //写缓冲区大小
        //http相应状态码
        enum HTTP_CODE
        {
            NO_REQUEST,
            GET_REQUEST,
            BAD_REQUEST,
            NO_RESOURSE,
            FORBIDDEN_REQUEST,
            FILE_REQUEST,
            INTERNAL_ERROR,
            CLOSED_CONNECTION
        };
        //处理http请求时所处的不同阶段，主状态机
        enum CHECK_STATE
        {
            CHECK_STATE_REQUESTLINE,    //当前正在分析请求行
            CHECK_STATE_HEADER,         //当前正在分析头部(首部行)
            CHECK_STATE_CONTENT         //当前正在分析实体主体
        };
        //行的读取状态，从状态机的三种状态
        enum LINE_STATUS
        {
            LINE_OK,    //读取到一个完整的行
            LINE_BAD,   //行出错
            LINE_OPEN   //行数据不完整
        };
        //http请求方法
        enum METHOD
        {
            GET,
            POST,
            HEAD,
            PUT,
            DELETE,
            TRACE,
            OPTIONS,
            CONNECT,
            PATH
        };
    public:
        http_conn(){}   //构造函数
        ~http_conn(){}  //析构函数
        //初始化连接，外部调用
        void init(int sockfd, const sockaddr_in &addr, char* root, int trigmode, int close_log, string user, string passwd, string sqlname);
        bool read_once();   //从连接套接字中读取一次数据
        bool write();   //向连接套接字写入数据

        void process(); //处理客户请求
        void close_conn(bool real_close=true);  //关闭连接

    public:
        int m_epollfd;  //epoll文件描述符
        static int m_user_count;    //已连接用户数量
        int improv; //改进标志
        int timer_flag; //定时器标志
        int m_state;    //读写标记，标记子线程要处理的任务类型，0为读，1为写
    
    private:
        //连接相关属性

        int m_sockfd;   //连接套接字
        sockaddr_in m_addr; //客户端连接地址

        char m_read_buf[READ_BUFFER_SIZE];  //读缓冲区
        long m_read_idx;    //读缓冲区索引
        long m_checked_idx; //已检查过的数据索引
        char m_write_buf[WRITE_BUFFER_SIZE];    //写缓冲区
        int m_write_idx;    //写缓冲区索引
        int m_start_line;   //http请求行在读缓冲区中的位置
        int bytes_to_send;  //要发送的字节数
        int bytes_haved_send;   //已发送的字节数

        int m_trigmode;  //事件触发模式标志，0为LT，1为ET
        bool m_linger;   //http请求是否启用持久连接

        int cgi;    //是否启用POST
        char* doc_root; //服务器文档根目录
        char* m_file_address;   //要发送的文件映射到内存的起始地址
        struct stat m_file_stat;    //存储请求的文件属性信息

        //用于分散或聚集I/O操作，每个iovec都记录了一个缓冲区
        //m_iv[0]存储了待发送的http头部这一固定数据，即写缓冲区
        //m_iv[1]存储了待发送的http相应体中的文件内容，即要发送的文件映射到内存的地址
        struct iovec m_iv[2];
        int m_iv_count; //m_iv数组有效元素的数量

        //http处理相关
        CHECK_STATE m_check_state;  //主状态机的当前解析状态
        METHOD m_method; //请求行中解析出的请求方法
        int m_start_line;   //http开始行在读缓冲区中的起始位置
        char* m_url;    //请求行中解析出的url
        char* m_version;    //http版本
        char* m_content;    //存储请求的内容主体
        long m_content_length;   //请求的内容主体长度
        char* m_host;   //解析出的主机名
        char* m_real_file;  //请求的文件路径
        
    
    private:
        void init();    //初始化连接
        HTTP_CODE process_read();   //读取解析客户端的http请求并返回对应的状态码
        bool process_write(HTTP_CODE ret);  //根据http请求的处理结果构建http响应报文
        LINE_STATUS parse_line();   //解析一行的内容，从状态机
        char* get_line(){return m_read_buf+m_start_line;}   //获取读缓冲区中http报文的一行
        HTTP_CODE parse_request_line(char* text); //解析http请求行，获取请求方法、url、http版本号
        HTTP_CODE parse_headers(char* text);    //解析http首部行
        HTTP_CODE parse_content(char* text);    //判断http请求的主体是否被完整读入
        HTTP_CODE do_request(); //执行请求
        void unmap();   //解除请求文件的内存映射
        //响应构建
        bool add_response_line(const char* format, ...); //添加响应报文的一行
        bool add_status_line(int status, const char* title);    //添加状态行(http响应的开始行)
        bool add_headers(int content_length);   //添加首部行信息
        bool add_content_length(int content_length);    //添加首部行中Content-Length字段信息
        bool add_linger();  //添加首部行Connection字段信息
        bool add_blank_line();  //添加首部行中的空行
        bool add_content(const char* content);  //添加响应报文的内容主体
};
#endif