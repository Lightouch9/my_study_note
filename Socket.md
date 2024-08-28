# 通信流程
# 字节序

## 大端

## 小端

## 转换函数
Linux中提供如下4个函数进行主机字节序与网络字节序的转换：
```c
#include<netinet/in.h>
unsigned long int htonl(unsigned long int hostlong);
unsigned short int htons(unsigned short int hostshort); 
unsigned long int ntohl(unsigned long int netlong); 
unsigned short int ntohs(unsigned short int netshort);
```
从函数名上理解：
```c
htonl表示"host to network long"，即将长整型主机字节序准换为网络字节序
htons表示"host to network short"，即将短整型主机字节序转换为网络字节序
ntohl表示"network to host long"，即将长整型网络字节序转换为主机字节序
ntohs表示"network to host short"，即将短整型网络字节序转换为主机字节序
```
其中，长整型的两个函数通常用来转换ip地址，
短整型通常用来转换端口号。
# ip地址转换
## 只能转换ipv4的
为了更好的可读性，一般会将ip地址转换为字符串格式，比如点分十进制的ipv4地址，十六进制的ipv6地址。而在代码内部进行参数传递或者计算时，则又需要整数类型，所以需要一些函数来将ip地址在这两种类型之间进行转换。
```c
#include<arpa/inet.h>
//ipv4
//主机字节序转网络字节序
in_addr_t inet_addr(const char* strptr); 
int inet_aton(const char* cp,struct in_addr* inp); 
//网络字节序转主机字节序
char* inet_ntoa(struct in_addr in);
```
`inet_addr`用于将点分十进制字符串的ipv4（仅限ipv4）地址转换为网络字节序的ipv4地址，执行失败会返回`INADDR_NONE`。
`inet_aton`功能与`inet_addr`相同，但是会将转换的结果保存在参数`inp`指向的地址结构中。执行成功返回1，失败返回0。
`inet_ntoa`用于将网络字节序的ipv4（仅限ipv4）地址转换为点分十进制字符串的ipv4地址，但是该函数是**不可重入**的，其函数内部使用一个静态变量存储转换结果，函数返回值指向该静态内存。
```c
char*szValue1=inet_ntoa("1.2.3.4"); 
char*szValue2=inet_ntoa("10.194.71.60"); 
printf("address 1:%s\n",szValue1); 
printf("address 2:%s\n",szValue2);
```
## ipv4与ipv6通用
使用的头文件同上`<arpa/inet.h>`

主机字节序转网络字节序：
```c
int inet_pton(int af, const char* src, void* dst);
```
`af`参数用于指定地址族，`AF_INET`为ipv4，`AF_INET6`为ipv6。
`src`则是要转换的ip地址的字符串。
`dst`则是转换结果的存储位置。
执行成功返回1，失败返回0并设置errno

网络字节序转主机字节序：
```c
const char* inet_ntop(int af, const void* src, char* dst, socklen_t size);
```
`inet_ntop`前三个参数同上，`size`参数指定结果存储内存的大小，可以直接使用以下宏值：
```c
#include<net/in.h>
#define INET_ADDRSTRLEN 16    //对应ipv4
#define INET6_ADDRSTRLEN 46   //对应ipv6
```
执行成功返回结果存储内存的地址，失败返回NULL并设置errno
# 通用socket地址
## sockaddr
sockaddr是一种结构体，用于表示socket地址，数据结构定义：
```c
#include<bits/socket.h> 
struct sockaddr 
{ 
	sa_family_t sa_family; 
	char sa_data[14]; 
}
```
成员解释：
`sa_family`表示地址族协议类型`sa_family_t`成员，该成员有以下宏值：
```
AF_UNIX:UNIX本地域协议族
AF_INET:TCP/IPv4协议族
AF_INET6:TCP/IPv6协议族
```
这些宏值定义在`<bits/socket.h>`中，通常使用`AF_INET`。
`sa_data`则用来存放socket地址的ip与端口号，如果使用ipv4，14字节由端口(2字节)，ip地址(4字节)，填充(8字节)组成。不同协议族具体结构如下：
```
AF_UNIX:文件的路径名，长度可达108字节
AF_INET:端口号2字节，ip地址4字节
AF_INET6:端口号2字节，流标识4字节，ip地址16字节，范围ID4字节，共26字节
```
## sockaddr_storage
可以看出`sockaddr`的`sa_data`成员的14字节的大小并不能完全满足所有的协议族，所以Linux中定义了新的通用socket地址结构体：
```c
#include<bits/socket.h>
struct sockaddr_storage 
{ 
	sa_family_t sa_family; 
	unsigned long int__ss_align; 
	char __ss_padding[128-sizeof(__ss_align)]; 
}
```
使用`unsigned long`类型存储socket地址，提供了足够大的空间，同时`__ss_padding`使得这是内存对齐的。
# 专用socket地址
但是以上两个通用socket地址结构体在获取与设置上涉及繁琐的位操作，于是Linux中提供了用于各个协议族的专用socket地址结构体。
## UNIX本地域协议族`sockaddr_un`
```c
#include<sys/un.h>
struct sockaddr_un 
{ 
	sa_family_t sin_family;/*地址族：AF_UNIX*/ 
	char sun_path[108];/*文件路径名*/ 
};
```
## TCP/IP协议族`sockaddr_in`和`sockaddr_in6`
`sockaddr_in`用于ipv4，`sockaddr_in6`用于ipv6
```c
struct sockaddr_in 
{ 
	sa_family_t sin_family;/*地址族：AF_INET*/ 
	u_int16_t sin_port;/*端口号，要用网络字节序表示*/ 
	struct in_addr sin_addr;/*IPv4地址结构体，见下面*/ 
}; 
struct in_addr 
{ 
	u_int32_t s_addr;/*IPv4地址，要用网络字节序表示*/ 
}; 
struct sockaddr_in6 
{ 
	sa_family_t sin6_family;/*地址族：AF_INET6*/ 
	u_int16_t sin6_port;/*端口号，要用网络字节序表示*/ 
	u_int32_t sin6_flowinfo;/*流信息，应设置为0*/ 
	struct in6_addr sin6_addr;/*IPv6地址结构体，见下面*/ 
	u_int32_t sin6_scope_id;/*scope ID，尚处于实验阶段*/ 
}; 
struct in6_addr 
{ 
	unsigned char sa_addr[16];/*IPv6地址，要用网络字节序表示*/ 
};
```
所有的专用socket地址类型以及通用socket地址类型`sockaddr_storage`在使用时需要强制转换为通用socket地址类型`sockaddr`，因为所有socket编程接口使用的地址参数类型都是`sockaddr`。
# 套接字

套接字创建，返回值是文件描述符，通过该描述符操作内核中的一块内存，用于网络通信。：

```c
int socket(int domain, int type, int protocol);
```

将文件描述符与本地ip与端口绑定：

```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

设置监听：

```c
int listen(int sockfd, int backlog);
```

等待并接受连接：

```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

接收数据：

```c
ssize_t read(int sockfd, void *buf, size_t size);
ssize_t recv(int sockfd, void *buf, size_t size, int flags);
```

发送数据：

```c
ssize_t write(int fd, const void *buf, size_t len);
ssize_t send(int fd, const void *buf, size_t len, int flags);
```

客户端发起连接请求：

```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

# 网络信息API
```c
#include<netdb.h>
//根据主机名获取主机完整信息
struct hostent*gethostbyname(const char*name);
//根据ip地址获取主机完整信息
struct hostent*gethostbyaddr(const void*addr,size_t len,int type);

//根据名称获取服务完整信息
struct servent*getservbyname(const char*name,const char*proto); 
//根据端口获取服务完整信息
struct servent*getservbyport(int port,const char*proto);

//既能通过主机名获取ip地址，也能通过服务名获取端口号
int getaddrinfo(const char*hostname,const char*service,
				const struct addrinfo*hints,struct addrinfo**result);

//通过socket地址同时获取字符串形式的主机名和服务名
int getnameinfo(const struct sockaddr*sockaddr,
				socklen_t addrlen,char*host,socklen_t hostlen,char*serv,
				socklen_t servlen,int flags);
```
# 高级I/O函数
## 创建文件描述符
### pipe
创建管道
```c
#include<unistd.h>
int pipe(int fd[2]);
```
### dup/dup2
文件描述符复制重定向
```c
#include<unistd.h>
int dup(int file_descriptor);
int dup2(int file_descriptor_one, int file_descriptor_two);
```
## 读写数据函数
### readv/writev
数据分散读，集中写
```c
#include<sys/uio.h>
ssize_t readv(int fd, const struct iovec* vector, int count);
ssize_t writev(int fd, const struct iovec* vector, int count);
```
### sendfile
在两个文件描述符之间直接传递数据
```c
#include<sys/sendfile.h>
ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count);
```
`out_fd`必须是指向真实文件，`in_fd`必须是socket
### mmap/munmap
申请与释放内存空间
```c
#include<sys/mman.h>
void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void* start, size_t length);
```
### splice
在两个文件描述符之间移动数据，零拷贝
```c
#include<fcntl.h>
ssize_t splice(int fd_in, loff_t* off_in, int fd_out, loff_t* off_out, size_t len,
			   unsigned int flags);
```
### tee
在两个管道文件描述符之间复制数据，零拷贝
```c
#include<fcntl.h>
ssize_t tee(int fd_in, int fd_ot, size_t len, unsigned int flags);
```
## 控制I/O行为与属性的函数
### fcntl
file control，提供对文件描述符的各种控制操作
```c
#include<fcntl.h>
int fcntl(int fd, int cmd,...);
```
# 其他
## 改变工作目录和根目录
获取当前工作目录和改变工作目录
```c
#include<unistd.h>
//获取当前工作目录
char* getcwd(char* buf, size_t size);
//改变工作目录
int chdir(const char* path);
```
改变进程根目录，特权进程才能改变根目录
```c
#include<unistd.h>
int chroot(const char* path);
```
## 程序后台化
```c
#include<unistd.h>
int daemon(int nochdir, int noclose);
```
# 服务器模型
## C/S
## P2P
# 服务器程序基本框架
I/O处理单元--->请求队列--->逻辑单元--->请求队列--->网络存储单元
# I/O模型
阻塞I/O
非阻塞I/O
socket基础api中可能被阻塞的系统调用包括accept,send,recv,connect
I/O复用
信号
同步I/O 异步I/O
# 事件处理模式
## Reactor
同步I/O
## Proactor
异步I/O
# 并发模式
## 半同步/半异步
同步：程序完全按照代码序列的顺序执行；
异步：程序的执行需要由系统事件驱动。

同步线程用于处理客户逻辑，相当于逻辑单元；
异步线程用于处理I/O事件，相当于I/O处理单元。
## 领导者/追随者
# 有限状态机
http请求分析
# I/O复用
同时监听多个文件描述符
## select
在一段指定的时间内，监听用户感兴趣的文件描述符上的可读、可写、异常事件
```c
#include<sys/select.h>
int select(int nfds,fd_set* readfds,fd_set* writefds,fd_set* exceptfds,
		   struct timeval* timeout)
```