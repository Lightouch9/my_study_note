#include<fstream>
#include"http_conn.h"

//定义http响应的状态信息
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_from = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "权限不足，禁止访问。。。\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "请求资源未找到。。。\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the request file.\n";
//初始化连接，外部调用
void http_conn::init(int sockfd, const sockaddr_in &addr, char* root, int trigmode)
{
    m_sockfd=sockfd;
    m_addr=addr;
    //向epoll事件表注册读事件，设置EPOLLONESHOT，ET触发模式
    addfd(m_epollfd, sockfd, true, m_trigmode);
    //更新用户计数
    m_user_count++;
    m_trigmode=trigmode;

    init();
}
//初始化新接受的连接
void http_conn::init()
{
    bytes_to_send=0;
    bytes_haved_send=0;
    m_check_state=CHECK_STATE_REQUESTLINE;
    m_linger=false;
    m_method=GET;
    m_url=0;
    m_version=0;
    m_content_length=0;
    m_host=0;
    m_start_line=0;
    m_checked_idx=0;
    m_read_idx=0;
    m_write_idx=0;
    cgi=0;
    m_state=0;
    timer_flag=0;
    improv=0;
    //缓冲区初始化
    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}
//从epoll事件表中移除文件描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}
//设置文件描述符的非阻塞
int setnonblocking(int fd)
{
    int old_option=fcntl(fd, F_GETFL);  //获取修改前的状态标志
    int new_option=old_option | O_NONBLOCK; //设置新的状态标志
    fcntl(fd, F_SETFL, new_option);
    return old_option;  //返回旧的状态标志
}
//向epoll事件表注册读事件，并设置非阻塞
void addfd(int epollfd, int fd, bool one_shot, int trigmode)
{
    //构建epoll_event
    epoll_event event;
    event.data.fd=fd;
    //ET模式
    if(trigmode==1)
        event.events=EPOLLIN | EPOLLET | EPOLLRDHUP;
    //LT模式
    else
        event.events=EPOLLIN | EPOLLRDHUP;
    //判断是否开启EPOLLONESHOT
    if(one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, event);
    setnonblocking(fd);
}
//关闭连接
void http_conn::close_conn(bool real_close=true)
{
    if(real_close&&(m_sockfd!=-1))
    {
        removefd(m_epollfd, m_sockfd);
        m_sockfd=-1;    //重置连接文件描述符
        m_user_count--; //连接用户数-1
    }
}
//修改epoll事件表中的某个文件描述符的事件并添加EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int trigmode)
{
    //构建修改用事件表
    epoll_event event;
    event.data.fd=fd;
    //ET
    if(trigmode==1)
        event.events=ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    //LT
    else
        event.events=ev | EPOLLONESHOT | EPOLLRDHUP;
    
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}
//解除请求文件的内存映射
void http_conn::unmap()
{
    if(m_file_address)
    {
        //释放内存空间
        munmap(m_file_address, m_file_stat.st_size);
        //内存重置
        m_file_address=0;
    }
}
//从连接套接字中读取一次数据
bool http_conn::read_once()
{
    //循环读取客户数据，直到无数据可读或对方关闭连接
    //非阻塞ET工作模式下，需要一次性将数据读完

    //读缓冲区索引合法性检查
    if(m_read_idx>=READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read; //recv返回值接受参数，记录读取到的字节数
    //LT模式
    if(m_trigmode==0)
    {
        //从连接套接字中读取数据到读缓冲区
        bytes_read=recv(m_sockfd, m_read_buf+m_read_idx, READ_BUFFER_SIZE-m_read_idx, 0);
        //更新读索引的值
        m_read_idx+=bytes_read;
        //检查recv返回值
        if(bytes_read<=0)
        {
            return false;
        }
        return true;
    }
    //ET模式
    else
    {
        while(true)
        {
            //接受数据
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            //错误处理
            if(bytes_read==-1)
            {
                //无数据可读，已读取完毕，中断循环
                if(errno==EAGAIN || errno==EWOULDBLOCK)
                    break;
                return false;
            }
            //连接已关闭
            else if(bytes_read==0)
            {
                return false;
            }
            //更新读缓冲区索引
            m_read_idx += bytes_read;
        }
        return true;
    }
}
//向连接套接字写入数据
bool http_conn::write()
{
    int temp=0;
    //要发送的数据的字节数为0
    if(bytes_to_send==0);
    {
        //将监听事件转为读事件
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_trigmode);
        //初始化连接
        init();
        return true;
    }
    //发送数据
    while(true)
    {
        //集中写
        temp=writev(m_sockfd, m_iv, m_iv_count);
        //错误处理
        if(temp<0)
        {
            if(errno==EAGAIN)
            {
                //当前不可写
                //设置监听事件为写事件，等待可写时在发送
                modfd(m_epollfd, m_sockfd, EPOLLOUT, m_trigmode);
                return true;
            }
            //解除内存映射返回错误
            unmap();
            return false;
        }
        //更新发送状态
        bytes_haved_send+=temp;
        bytes_to_send-=temp;
        //如果m_iv[0]的数据已经发送完成
        if(bytes_haved_send>=m_iv[0].iov_len)
        {
            m_iv[0].iov_len=0;
            //调整m_iv[1]缓冲区的起始地址和要发送数据长度
            m_iv[1].iov_base=m_file_address+(bytes_haved_send-m_write_idx);
            m_iv[1].iov_len=bytes_to_send;
        }
        //否则
        else
        {
            m_iv[0].iov_base=m_write_buf+bytes_haved_send;
            m_iv[0].iov_len-=bytes_haved_send;
        }
        //发送完毕
        if(bytes_to_send<=0)
        {
            //解除映射释放资源
            unmap();
            //监听事件设置为读事件
            modfd(m_epollfd, m_sockfd, EPOLLIN, m_trigmode);
            //如果延迟关闭
            if(m_linger)
            {
                //初始化连接
                init();
                return true;
            }
            return false;
        }
    }
}
//解析一行的内容，从状态机，返回行读取状态
http_conn::LINE_STATUS http_conn::parse_line()
{
    char temp;
    //逐字节分析读缓冲区的数据
    for(;m_checked_idx<m_read_idx;m_checked_idx++)
    {
        //获取当前要分析的字节
        temp=m_read_buf[m_read_idx];
        //读取到一个回车符，则可能读取到一个完整的行
        if(temp=='/r')
        {
            //如果这个'/r'正好是缓冲区中最后一个字符，则本次读取为不完整的行，需要进一步读取数据
            if(m_read_buf[m_checked_idx+1]==m_read_idx)
                return LINE_OPEN;
            //下一个字符如果是'/n'则是完整的行
            else if(m_read_buf[m_checked_idx+1]=='/n')
            {
                //将行结束的标记设置为结束符
                m_read_buf[m_checked_idx++]='\0';
                m_read_buf[m_checked_idx++]='\0';
                return LINE_OK;
            }
            //否则客户发送的请求有语法问题
            return LINE_BAD;
        }
        //读取到一个换行符也可能读取到一个完整行
        else if(temp=='/n')
        {
            if(m_checked_idx>1&&m_read_buf[m_checked_idx-1]=='r')
            {
                //将行结束的标记设置为结束符
                m_read_buf[m_checked_idx-1]='\0';
                m_read_buf[m_checked_idx++]='\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    //如果遍历完当前的读缓冲区都没有'/r'则行不完整
    return LINE_OPEN;
}
//解析http请求行，获取请求方法、url、http版本号
http_conn::HTTP_CODE http_conn::parse_request_line(char* text)
{
    //匹配url
    m_url=strpbrk(text, " \t");
    //错误处理
    if(!m_url)
    {
        //错误的请求
        return BAD_REQUEST;
    }
    //将请求行中的请求方法与url分离
    *m_url++='\0';
    //获取请求方法
    char* method=text;
    //判断请求方法
    if(strcasecmp(method, "GET")==0)
        m_method=GET;
    else if(strcasecmp(method, "POST")==0)
    {
        m_method=POST;
        //cgi=1;
    }
    else
        return BAD_REQUEST;
    //匹配url，找到第一个不是空格或制表符的位置
    m_url+=strspn(m_url, " \t");
    //匹配http版本
    m_version=strpbrk(m_url, " \t");
    if(strcasecmp(m_version, "HTTP/1.1")!=0);
        return BAD_REQUEST;
    //去除url中的前缀http://
    if(strcasecmp(m_url, "http://", 7)==0)
    {
        //如果m_url的前7个字符与http://相等
        m_url+=7;
        //取得路径
        m_url=strchr(m_url, '/');
    }
    //去除url中的前缀https://
    if(strcasecmp(m_url, "https://", 8)==0)
    {
        //如果m_url的前8个字符与https://相等
        m_url+=8;
        //取得路径
        m_url=strchr(m_url, '/');
    }
    if(!m_url || m_url[0]!='/')
        return BAD_REQUEST;
    //当请求的路径为空时
    if(strlen(m_url)==1)
        strcat(m_url, "judge.html");    //将路径修改为判断界面
    m_check_state=CHECK_STATE_HEADER;
    return NO_REQUEST;
}
//解析http首部行
http_conn::HTTP_CODE http_conn::parse_headers(char* text)
{
    //如果是空行，则解析完毕
    if(text[0]=='\0')
    {
        //如果有内容主体，则将状态机转换为CHECK_STATE_CONTENT
        if(m_content_length!=0)
        {
            m_check_state=CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    //Connection字段
    else if(strncasecmp(text, "Connection:", 11)==0)
    {
        //调整text到该字段的值的部分
        text+=11;
        text+=strspn(text, " \t");
        //如果使用持久连接
        if(strcasecmp(text, "keep-alive")==0)
        {
            m_linger=true;
        }
    }
    //Content-Length字段
    else if(strncasecmp(text, "Content-Length:", 15)==0)
    {
        //调整text到该字段的值的部分
        text+=15;
        text+=strspn(text, " \t");
        //获取内容主体长度
        m_content_length=atol(text);    //转换为长整型
    }
    //Host字段
    else if(strncasecmp(text, "Host:", 5)==0)
    {
        //调整text到该字段的值的部分
        text+=5;
        text+=strspn(text, " \t");
        //获取内容主体长度
        m_host=text;    //转换为长整型
    }
    //其他字段
    else
    {
        //记入日志
    }
    return NO_REQUEST;
}
//判断http请求的主体是否被完整读入
http_conn::HTTP_CODE http_conn::parse_content(char* text)
{
    if(m_read_idx>=(m_checked_idx+m_content_length))
    {
        text[m_content_length]='\0';
        //POST请求用户名与密码存放于消息主体中
        m_content=text;
        return GET_REQUEST; //已获取完整请求
    }
    return NO_REQUEST;
}
//读取解析客户端的http请求并返回对应的状态码
http_conn::HTTP_CODE http_conn::process_read()
{
    LINE_STATUS line_status=LINE_OK;    //初始化行的读取状态
    HTTP_CODE ret=NO_REQUEST;   //初始化状态码返回值
    char* text=0;   //存储读取到的信息

    while((m_check_state==CHECK_STATE_CONTENT && line_status==LINE_OK)||((line_status=parse_line())==LINE_OK))
    {
        //获取新的一行内容
        text=get_line();
        //更新m_start_line
        m_start_line=m_checked_idx;
        switch(m_check_state)
        {
            //解析请求行
            case CHECK_STATE_REQUESTLINE:
            {
                ret=parse_request_line(text);
                if(ret==BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            //解析请求的首部行
            case CHECK_STATE_HEADER:
            {
                ret=parse_headers(text);
                if(ret==BAD_REQUEST)
                    return BAD_REQUEST;
                //如果已经得到了一个完整的请求则执行请求
                else if(ret==GET_REQUEST)
                {
                    return do_request();
                }
                break;
            }
            //解析请求内容主体
            case CHECK_STATE_CONTENT:
            {
                ret=parse_content(text);
                if(ret==GET_REQUEST)
                    return do_request();
                line_status=LINE_OPEN;
                break;
            }
            default:
                //无法解析请求
                return INTERNAL_ERROR;
        }

    }
    return NO_REQUEST;
}
//执行请求
http_conn::HTTP_CODE http_conn::do_request()
{
    //获取服务器文档根目录
    strcpy(m_real_file, doc_root);
    int len = strlen(doc_root); //服务器根目录文档路径长度
    //获取请求url中最后一个'/'用于确定请求的类型
    const char* p=strrchr(m_url, '/');
    //处理cgi
    if(cgi==1 && (*(p+1)=='2' || *(p+1)=='3'))
    {
        //获取标志判断是登录检测还是注册检测
        char flag = m_url[1];
        //根据请求url构建要访问的文件路径
        char* m_url_real= (char *)malloc(sizeof(char)* 200);    //保存要访问的文件的路径的一部分
        //构建url路径
        strcpy(m_url_real, '/');    //根目录
        strcat(m_url_real, m_url+2);    //将剩余目录拼接上，从url第三个字符开始
        strncpy(m_real_file + len, m_url_real, FILENAME_LEN-len-1 );    //构建完整请求的文档目录
        free(m_url_real);   //释放作为工具的中间变量

        //提取用户名和密码
        //user=123&password=123
        char name[100]; //存储用户名
        char passwd[100];   //存储密码
        //提取用户名
        int i;
        for(i=5;m_content[i]!='&';i++)  //从第6个字符开始判断
            name[i-5]=m_content[i];
        name[i-5]='\0'; //设置结束符
        //提取密码
        int j=0;
        for(i=i+10;m_content[i]!='\0';i++, j++)
            passwd[j]=m_content[i];
        passwd[j]='\0'; //设置结束符

        //注册处理
        if(*(p+1)=='3')
        {
            //先进行重名检测
            //构建sql插入语句
        }
        //登录处理
        else if(*(p+1)=='2')
        {
            //检查用户名与密码是否匹配
            if()
            {
                //匹配则将m_url设置为欢迎界面/welcome.html
                //否则m_url为错误界面/logError.html
            }
        }
    }
    //其他请求
    //注册页面
    if(*(p+1)=='0')
    {
        //构建注册页面的文件目录地址
        char* m_url_real= (char *)malloc(sizeof(char)* 200);    //保存要访问的文件的路径的一部分
        strcpy(m_url_real, "/register.html");
        strncpy(m_real_file+len, m_url_real, strlen(m_url_real));
        free(m_url_real);
    }
    //登录界面
    if(*(p+1)=='1')
    {
        //构建登录页面的文件目录地址
        char* m_url_real= (char *)malloc(sizeof(char)* 200);    //保存要访问的文件的路径的一部分
        strcpy(m_url_real, "/log.html");
        strncpy(m_real_file+len, m_url_real, strlen(m_url_real));
        free(m_url_real);
    }
    //图片页面
    else if(*(p+1)=='5')
    {
        //构建图片页面的文件目录地址
        char* m_url_real= (char *)malloc(sizeof(char)* 200);    //保存要访问的文件的路径的一部分
        strcpy(m_url_real, "/picture.html");
        strncpy(m_real_file+len, m_url_real, strlen(m_url_real));
        free(m_url_real);
    }
    //视频页面
    else if(*(p+1)=='6')
    {
        //构建注册页面的文件目录地址
        char* m_url_real= (char *)malloc(sizeof(char)* 200);    //保存要访问的文件的路径的一部分
        strcpy(m_url_real, "/video.html");
        strncpy(m_real_file+len, m_url_real, strlen(m_url_real));
        free(m_url_real);
    }
    //请求其他
    else
        strncpy(m_real_file+len, m_url, FILENAME_LEN-len-1);
    //检查请求的文件有效性
    if(stat(m_real_file, &m_file_stat)<0)
        //文件不存在
        return NO_RESOURSE;
    if(!(m_file_stat.st_mode&S_IROTH))
        //文件不可读
        return FORBIDDEN_REQUEST;
    if(S_ISDIR(m_file_stat.st_mode))
        //请求的是目录
        return BAD_REQUEST;
    //映射请求的文件映射到内存
    int fd=open(m_real_file, O_RDONLY); //只读
    m_file_address=(char*)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}
//响应构建
//添加响应报文的一行
bool http_conn::add_response_line(const cahr* format, ...)
{
    //写缓冲区已满
    if(m_write_idx>=WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;   //变长参数列表
    va_start(arg_list, format); //初始化参数列表
    //将传入内容写入缓冲区
    int len=vsnprintf(m_write_buf+m_write_idx, WRITE_BUFFER_SIZE-m_write_idx-1, format, arg_list);
    //写入结果检查
    if(len>=(WRITE_BUFFER_SIZE-m_write_idx-1))
    {
        //发生截断
        va_end(arg_list);
        return false;
    }
    m_write_idx+=len;
    va_end(arg_list);
    return true;
}
//添加状态行
bool http_conn::add_status_line(int status, const char* title)
{
    return add_response_line("%s %d %s\r\n", "HTTP/1.1", status, title);
}
//添加首部行信息
bool http_conn::add_headers(int content_length)
{
    return add_content_length(content_length) && add_linger() && add_blank_line();
}
//添加首部行中Content-Length字段信息
bool http_conn::add_content_length(int content_length)
{
    return add_response_line("Content-Length:%d\r\n", content_length);
}
//添加首部行Connection字段信息
bool http_conn::add_linger()
{
    add_response_line("Connection:%s\r\n", (m_linger==true)? "keep-alive":"close");
}
//添加首部行中的空行
bool http_conn::add_blank_line()
{
    add_response_line("%s", "\r\n");
}
//添加响应报文的内容主体
bool http_conn::add_content(const char* content)
{
    return add_response_line("%s", content);
}
//根据http请求的处理结果构建http响应报文
bool http_conn::process_write(HTTP_CODE ret)
{
    //根据状态码构建不同的响应报文
    switch(ret)
    {
        //服务器内部错误
        case INTERNAL_ERROR:
        {
            add_status_line(500, error_500_title);
            add_headers(strlen(error_500_form));
            if(!add_content(error_500_form))
                return false;
            break;
        }
        //请求资源未找到
        case BAD_REQUEST:
        {
            add_status_line(404, error_404_title);
            add_headers(strlen(error_404_form));
            if(!add_content(error_404_form));
                return false;
            break;
        }
        //禁止访问
        case FORBIDDEN_REQUEST:
        {
            add_status_line(403, error_403_title);
            add_headers(strlen(error_403_form));
            if(!add_content(error_403_form));
                return false;
            break;
        }
        //成功访问
        case FILE_REQUEST:
        {
            add_status_line(200, ok_200_title);
            //请求的文件大小不为0
            if(m_file_stat.st_size!=0)
            {
                add_headers(m_file_stat.st_size);
                //存放响应首部信息
                m_iv[0].iov_base=m_write_buf;
                m_iv[0].iov_len=m_write_idx;
                //存放要发送的文件内容
                m_iv[1].iov_base=m_file_address;
                m_iv[1],iov_len=m_file_stat.st_size;
                //待发送的字节数
                bytes_to_send=m_write_idx+m_file_stat.st_size;
                return true;
            }
            else
            {
                const char* ok_string="<html><body></body></html>";
                add_headers(strlen(ok_string));
                if(!add_content(ok_string))
                    return false;
            }
        }
        default:
            return false;
    }
    //设置要发送的http首部行，非成功访问情况下
    m_iv[0].iov_base=m_write_buf;
    m_iv[0].iov_len=m_write_idx;
    m_iv_count=1;
    bytes_to_send=m_write_idx;
    return true;
}
//处理客户请求
void http_conn::process()
{
    //读取并解析http请求报文
    HTTP_CODE read_ret=process_read();
    //如果请求未完全读取
    if(read_ret==NO_REQUEST)
    {
        //继续监听读事件
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_trigmode);
        return;
    }
    //生成http响应
    bool write_ret=process_write(read_ret);
    //生成响应失败
    if(!write_ret)
    {
        //关闭连接
        close_conn()
    }
    //成功,监听写事件，准备向客户端发送数据
    modfd(m_epollfd, m_sockfd, EPOLLIN, m_trigmode);
}