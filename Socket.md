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
所有的专用socket地址类型以及通用socket地址类型`sockaddr_storage`在使用时需要**强制转换为通用socket地址类型`sockaddr`**，因为所有socket编程接口使用的地址参数类型都是`sockaddr`。
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
在一段指定的时间内，监听多个文件描述符上的状态变化（可读、可写、异常事件）。
`select`可以减少程序的I/O等待时间，因为它允许程序在等待一个文件描述符变为就绪状态时，同时处理其他文件描述符的I/O操作。
```c
#include<sys/select.h>
int select(int nfds,fd_set* readfds,fd_set* writefds,fd_set* exceptfds,
		   struct timeval* timeout)
```
- `nfds`：指定被监听的文件描述符的总数，通常设置为`select`监听的所有文件描述符中的最大值加1，因为文件描述符是从0开始计数的。
- `readfds`,`writefds`,`exceptfds`：分别指向可读、可写、异常事件对应的文件描述符集合`fd_set`。`select`调用返回时会修改这3个参数来通知程序哪些文件描述符已经就绪。
```c
//fd_set结构体定义
#include<typesizes.h＞>
#define __FD_SETSIZE 1024 
#include<sys/select.h>
#define FD_SETSIZE __FD_SETSIZE 
typedef long int__fd_mask; 
#undef __NFDBITS 
#define __NFDBITS(8*(int)sizeof(__fd_mask)) 
typedef struct 
{ 
#ifdef__USE_XOPEN __fd_mask fds_bits[__FD_SETSIZE/__NFDBITS]; #define__FDS_BITS(set)((set)->fds_bits) 
#else 
__fd_mask__fds_bits[__FD_SETSIZE/__NFDBITS]; 
#define__FDS_BITS(set)((set)->__fds_bits) 
#endif 
}fd_set;
```
`fd_set`中通过一个整形数组记录文件描述符集合，容量大小由`FD_SETSIZE`指定，限制了`select`同时处理的文件描述符的总量。
- `timeout`：用来设置`select`函数的超时时间，类型为指向`timeval`结构类型的指针。`timeval`结构体定义如下：
```c
struct timeval
{
	//秒
	long tv_sec;
	//微秒
	long tv_usec;
}
```
由此可见`select`调用的超时等待可以精确到微秒级，如果`tv_sec`与`tv_usec`均设置为0，则`select`立即返回，而如果给`timeout`传入NULL，则`select`会一直阻塞，直到某个文件描述符就绪。
`select`执行成功返回就绪的文件描述符的总数，执行失败返回-1并设置errno，如果`select`在等待的期间接收到信号，则立即返回-1并设置errno为`EINTR`。

## poll
在指定时间内轮询一定数量的文件描述符，检测其中是否由就绪的。
```c
#include<poll.h>
int poll(struct pollfd* fds, nfds_t nfds, int timeout);
```
- `fds`是一个`pollfd`类型的数组，用于指定需要监听的文件描述符上的事件，`pollfd`定义如下：

  ```c
  /* Data structure describing a polling request.  */
  struct pollfd
    {
      int fd;			/* 要监听的文件描述符  */
      short int events;		/* 要关注的事件  */
      short int revents;		/* 实际发生的事件，此项由内核填充  */
    };
  ```

- `nfds`指定`fds`数组的大小，`nfds_t`类型定义如下：

  ```c
  /* Type used for the number of file descriptors.  */
  typedef unsigned long int nfds_t;
  ```

- `timeout`指定监听超时时间，单位毫秒，`timeout`为-1时，`poll`则在监听的时间发生前都将阻塞，`timeout`为0时，`poll` 将立即返回。

- `poll`的返回值同`select`。

## epoll

Linux特有的I/O复用函数，使用**一组函数**完成任务，`epoll`把用户关心的文件描述符上的事件放在内核里的一个事件表中，无需每次调用都传入文件描述符集或事件集，降低了内核空间与用户空间之间数据的拷贝开销，但需要传入一个标识这个事件表的文件描述符。
### epoll_create
创建事件表的文件描述符（创建`epoll`实例）：
```c
#include<sys/epoll.h>
int epoll_create(int size);
```
`size`用于提示内核事件表的大小，即此`epoll`实例所监听的文件描述符的数量上限，但通常此参数会被忽略，只需传入一个大于0的数即可。
执行成功会返回新创建的文件描述符，失败返回-1并设置errno。
### epoll_ctl
`epoll_ctl`用于操作`epoll`实例的内核事件表（由`epoll_create`返回）

```c
#include<sys/epoll.h>
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
```
- `epfd`：`epoll`实例的文件描述符
- `fd`：要操作的文件描述符
- `op`：操作类型，如下：
	- `EPOLL_CTL_ADD`：添加，向事件表中注册`fd`上的事件
	- `EPOLL_CTL_MOD`：修改，修改`fd`上的注册事件
	- `EPOLL_CTL_DEL`：删除，删除`fd`上的注册事件
- `event`：指定要关注的事件，其数据类型为指向`epoll_event`结构体的指针。
```c
//epoll_event
struct epoll_event
{
	//epoll事件类型
	__uint32_t events;
	//用户数据
	epoll_data_t data;
}
```
`epoll_ctl`执行成功返回0，失败返回-1并设置errno。
### epoll_wait
在一段指定的超时时间内返回就绪的文件描述符。
```c
#include<sys/epoll.h>
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, 
               int timeout);
```
当`epoll_wait`检测到事件就会将**所有就绪的**事件从`epfd`指定的内核事件表中复制到`events`指向的用于存放就绪事件的数组中，这个数组只用于输出`epoll_wait`检测到的就绪事件。
`maxevents`指定最多监听的事件的数量，即`events`数组的大小，
`timeout`指定超时时间，与`poll`的`timeout`参数含义相同。

### 工作模式LT与ET
LT(Level Trigger)，默认的工作模式，在此工作模式下当`epoll_wait`检测到文件描述符就绪并将其通知应用程序后，除非应用程序处理该事件，否则每次调用`epoll_wait`时都会向应用程序返回就绪的文件描述符。
ET(Edge Trigger)，此工作模式下，当检测到事件发生时只会通知应用程序一次，这要求应用程序必须立即处理该事件，因为后面不再通知该事件。同时要使用**非阻塞式**I/O操作。所以ET模式相对于LT模式能够降低同一个事件被`epoll`重复触发的次数，提高效率。

## 3个I/O复用函数的对比

`select`、`poll`、`epoll`三个I/O复用函数都能同时监听多个文件描述符，在等待由`timeout`指定的超时时间内，当有一个或多个文件描述符上有事件发生时返回，返回值为就绪的文件描述符的数量。

### 监听的文件描述符的传递

三个函数都通过一个结构体变量告诉内核需要监听的指定的文件描述符上的指定的事件，并通过该结构体参数来获取内核的处理结果。

- `select`通过`fd_set`文件描述符集合来指定监听的对象，但是`fd_set`没有将文件描述符和事件进行绑定，所以`select`需要3个`fd_set`分别指定可读、可写和异常事件。这使得不支持更多的事件类型，同时由于内核会修改`fd_set`，将其中就绪的文件描述符的状态设置为就绪对应的值，这使得当`select`返回时，`fd_set`已经不是传入的状态了，所以下次调用时需要重置`fd_set`。
- `poll`的`pollfd`参数则将文件描述符和事件进行了绑定，同时内核修改的是`pollfd`的`revents`成员，而`events`成员不变，因此下次调用`poll` 时无需重置事件集。
- `epoll`则通过在内核中维护一个事件表来管理用户注册的事件，并通过一个独立的系统调用`epoll_ctl`来对事件表进行修改(增、删、改)，这样`epoll_wait`就会直接从内核的事件表中取得用户注册的事件，这样当多次调用时就不用反复从用户空间将事件读入内核空间，减少了数据拷贝，提高了效率。
- `select`和`poll`在每次调用后会将所有事件的集合返回(包括就绪的和未就绪的)，所以应用程序对返回的结果进行索引就绪的文件描述符时时间复杂度为O(n)，而`epoll`的`epoll_wait`则通过`events`参数只返回就绪的文件描述符，所以应用程序对结果索引就绪的文件描述符的时间复杂度为O(1)。

### 最大监听数量限制

在文件描述符和事件的最大监听数量上三者也有所不同。

- `poll`和`epoll_wait`分别通过`nfds`和`maxevents`参数指定监听数量的最大值，这两个参数都能达到系统所限定的允许打开的最大文件描述符数目。
- `select`则在此限制上受到系统对单个进程可打开的文件描述符的限制，一般32位系统为1024，64位系统为2048，当然此限制用户可以通过系统进行修改，但容易导致不可预测的后果。

### 工作方式

工作方式上三者同样有所差别。

- `select`和`poll`采用轮询的方式，每次调用时会扫描整个文件描述符集合，所以在检测就绪文件描述符时时间复杂度为O(n)。
- `epoll_wait`则采用回调的方式，在检测到就绪的文件描述符时会将就绪的文件描述符上的事件插入到内核的就绪事件队列中，内核则将此队列中的内容拷贝到用户空间中，所以`epoll_wait`无需轮询所有事件，事件复杂度为O(1)。

但是当连接较多并且活动频繁时，`epoll_wait`未必比`select`和`poll`效率高，因为`epoll_wait`的回调函数触发的太频繁。

## I/O复用的应用：网络聊天室

此应用示例通过`poll` 为例实现一个简单的网络聊天室，利用I/O 复用技术同时处理网络连接和用户输入，实现所有用户同时在线群聊。

### 客户端程序

客户端程序需要实现两个功能：

- 从标准输入终端读入用户数据并将用户的数据发送到服务器
- 向标准输出终端打印从服务器接受到的数据

使用`poll`同时监听用户输入和网络连接，并利用`splice`将用户输入的内容直接定向到网络连接上发送，实现数据零拷贝，提高效率。

#### 头文件包含

```c
#define _GNU_SOURCE 1
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<poll.h>
#include<fcntl.h>
#include<libgen.h>
```

`#define _GNU_SOURCE 1`定义宏，启用GNU扩展。

这里包含了多种标准和`POSIX`头文件，用于网络编程、I/O操作、错误处理等。

#### 宏定义

```c
#define BUFFER_SIZE 64
```

定义一个大小为64字节的缓冲区。

#### 主函数

```c
int main(int argc, char* argv[]){}
```

程序执行入口，同时接受命令行参数。

#### 命令行参数检查

```c
if(argc<=2)
{
    printf("usage: %s ip_address port_number\n", basename(argv[0]));
    return 1;
}
```

检查程序通过命令行执行时是否提供了必要的参数，服务器ip地址与端口。

#### 设置服务器ip地址与端口

```c
//设置服务器ip与端口，从命令行读取
const char* ip=argv[1];
int port=atoi(argv[2]);
//创建存储服务器地址的结构体
struct sockaddr_in server_address;
//清空刚创建的server_address的内存块
bzero(&server_address, sizeof(server_address));
//设置地址族、ip、端口
server_address.sin_family=AF_INET;
inet_pton(AF_INET, ip, &server_address.sin_addr);
server_address.sin_port=htons(port);
```

> `atoi`函数(`<stdlib.h>`)`int atoi(const char* str)`用于将`str`指向的字符串转换为一个`int`整数。
>
> `bzero`函数(`<string.h>`)`void bzero(void* s, int n)`用于将`s`指向的内存块的前`n`个字节清零。

#### 创建套接字并连接

```c
//创建套接字
int sockfd=socket(AF_INET, SOCK_STREAM, 0);
assert(sockfd>=0);
if(connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address))<0)
{
    //如果连接失败打印错误信息
    printf("连接服务器失败。。。\n");
    close(sockfd);
    return 1;
}
```

> `assert`用来设置断言，位于`<assert.h>`中，`assert()`会检查传入的表达式是否为真或非零，为真则不做任何操作，为假或为零则显示错误信息并终止程序运行，一般在程序的开发和测试阶段进行逻辑错误的检查，不应用于正常的错误或异常处理。

#### 设置`poll`

```c
//设置pollfd用于poll
struct pollfd fds[2];
//注册文件描述符0，监听标准输入的可读事件
fds[0].fd=0;
fds[0].events=POLLIN;
fds[0].revents=0;
//注册文件描述符1，监听用于网络通信的文件描述符的可读与关闭事件
fds[1].fd=sockfd;
fds[1].events=POLLIN | POLLRDHUP;
fds[1].revents=0;
```

创建`pollfd`数组，监听标准输入与网络通信的套接字。

#### 创建管道

```c
//创建管道
int pipefd[2];
int ret=pipe(pipefd);
//设置断言
assert(ret!=-1);
```

#### 核心循环

这个循环持续处理网络连接和用户输入，直到发生错误或连接关闭。

```c
//创建读缓冲区
char read_buf[BUFFER_SIZE];
//核心循环
while(1)
{
    //阻塞监听标准输入与网络套接字
    ret=poll(fds, 2, -1);
    //执行错误，退出循环
    if(ret<0)
    {
        printf("poll监听错误。。。\n");
        break;
    }
    //处理服务器关闭套接字
    if(fds[1].revents & POLLRDHUP)
    {
        printf("服务器端关闭了连接。。。\n");
        break;
    }
    //处理服务器套接字是有数据可读，即服务器发送了数据
    else if(fds[1].revents & POLLIN)
    {
        //将读缓冲区清零，确保缓冲区没有干扰数据
        memset(read_buf, '\0', BUFFER_SIZE);
        //接受数据到缓冲区中
        recv(fds[1].fd, read_buf, BUFFER_SIZE-1, 0);
        //输出接收到的数据
        printf("%s\n", read_buf);
    }
    //处理用户输入事件
    if(fds[0].revents & POLLIN)
    {
        ret=splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        //若传输的数据量小于0，则出现了错误
        if(ret<0)
        {
            perror("splice");
            break;
        }
        //从管道的读端将数据写入sockfd发送给服务器
        ret=splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        //若传输的数据量小于0，则出现了错误
        if(ret<0)
        {
            perror("splice");
            break;
        }
    }
}
```

- 从网络套接字`sockfd`接受数据并读入缓冲区`read_buf`中时，读取的数据量的大小为`BUFFER_SIZE-1`，保留1个字节用于字符串的结束符`\0`。
- `splice`第一次调用时`splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);`，将数据从标准输入（文件描述符0）读取到管道的写端`pipefd[1]`，`32768`是尝试传输的字节数，标志位设置的`SPLICE_F_MORE`表示可能会有更多的数据传输，`SPLICE_F_MOVE`表示要尽可能的减少数据拷贝。
- `splice`第二次调用`ret=splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);`，将数据从管道的读端`pipefd[0]`写入到套接字`sockfd`中，即发送给服务器。

#### 关闭套接字

```c
//关闭套接字
close(sockfd);
return 0;
```

### 服务器端程序

服务器端负责通过`poll`同时管理监听`socket`和网络连接`socket`。

#### 头文件包含

```c
//头文件
#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>
```

这些头文件提供网络编程、I/O操作、内存管理等需要的函数和数据结构。

`#define _GNU_SOURCE 1`宏启用`GNU`扩展。

#### 宏定义

```c
//宏定义
//用户最大数量，即允许的最大并发连接数
#define USER_LIMIT 5
//读缓冲区大小
#define BUFFER_SIZE 64
//文件描述符数量限制，限制客户端连接数
#define FD_LIMIT 65535
```

#### 客户端信息数据结构定义

客户端信息数据结构用于存储客户端的地址信息和I/O缓冲区。

```c
//客户端信息
struct client_data
{
    //客户端socket地址
    sockaddr_in address;
    //写缓冲区，要写入客户端的数据
    char* write_buf;
    //读缓冲区，从客户端读入的数据
    char buf[BUFFER_SIZE];
}
```

#### 设置非阻塞函数定义

```c
//设置非阻塞模式函数
int setnonblocking(int fd)
{
    //获取当前状态标志
    int old_option=fcntl(fd, F_GETFL);
    //设置新的状态标志，添加非阻塞标志
    int new_option=old_option | O_NONBLOCK;
    //将新的状态标志添加进文件描述符
    fcntl(fd, F_SETFL, new_option);
    //返回旧标志集
    return old_option;
}
```

#### 监听套接字的创建

```c
//命令行参数检查
if(argc<=2)
{
    //检查是否提供必要的参数，服务器ip与端口
    printf("usage: %s ip_address port_number\n", basename(argv[0]));
    return 1;
}
//设置服务器ip与端口，从命令行读取
const char* ip=argv[1];
int port=atoi(argv[2]);
//设置服务器ip与端口，从命令行读取
const char* ip=argv[1];
int port=atoi(argv[2]);
//创建套接字地址的结构体
struct sockaddr_in address;
//清空刚创建的address的内存块
bzero(&address, sizeof(address));
//设置地址族、ip、端口
address.sin_family=AF_INET;
inet_pton(AF_INET, ip, &address.sin_addr);
address.sin_port=htons(port);
//创建监听套接字
int listenfd=socket(AF_INET, SOCK_STREAM, 0);
//断言
assert(sockfd>=0);
//绑定套接字到指定的ip地址和端口
int ret=bind(listenfd, (struct sockaddr*)&address, sizeof(address));
assert(ret>=0);
//设置监听状态
ret=listen(listenfd, 5);
assert(ret>=0);
```

#### 创建`client_data`数组和`pollfd`数组

```c
//创建client_data数组
struct client_data* users = (struct client_data*)malloc(FD_LIMIT * sizeof(struct client_data));
//创建pollfd数组
struct pollfd fds[USER_LIMIT+1];
//记录当前连接的客户端数量
int user_counter=0;
//初始化fds
for(int i=1; i<=USER_LIMIT;++i)
{
    fds[i].fd=-1;
    fds[i].events=0;
}
//fds[0]用于监听套接字
fds[0].fd=listenfd;
fds[0].events=POLLIN | POLLERR;
```

- `client_data`数组`users`的大小设置为65535，同时每个socket连接可以以其文件描述符作为数组下标来索引`client_data`数组以分配`client_data`对象。由于65535是文件描述符最大数量，这样每个可能的socket连接都能分配到`client_data`对象。
- `pollfd`数组`fds`用于描述要监视的文件描述符及其相关事件，而大小设置中的`+1`，则是用于监听描述符。

#### 处理新连接请求

```c
while(1)
{
    //监听fds中的文件描述符
    ret=poll(fds, user_counter+1, -1);
    if(ret<0)
    {
        printf("poll执行失败。。。\n");
        break;
    }
    for(int i=0;i<user_counter+1;++i)
    {
        //当有新连接请求时
        if((fds[i].fd=listenfd)&&(fds[i].events&POLLIN))
        {
            //接受新连接，创建与客户端通信的套接字
            struct sockaddr_in client_address;
            socklen_t client_addrlength=sizeof(client_address);
            int connfd=accept(listenfd, (struct sockaddr*)&client_address, &client_add
            if(connfd<0)
            {
                printf("发生错误：%d",errno);
                continue;
            }
            //如果连接数量达到限制，则关闭新连接
            if(user_counter>=USER_LIMIT)
            {
                const char* info="连接达到限制。。。\n";
                printf("%s", info);
                //向客户端发送反馈
                send(connfd, info, strlen(info), 0);
                close(connfd);
                continue;
            }
            //修改当前连接数量
            user_counter++;
            users[connfd].address=client_address;
            //将新连接设置为非阻塞
            setnonblocking(connfd);
            fds[user_counter].fd=connfd;
            //将新连接添加进fds进行监听
            fds[user_counter].fd=connfd;
            fds[user_counter].events=POLLIN | POLLRDHUP | POLLERR;
            fds[user_counter].revents=0;
            //输出当前连接情况
            printf("新连接已建立，当前连接数量：%d\n", user_counter);
            
        }
    }
}
```

#### 处理错误信息

```c
//处理错误信息
else if(fds[i].revents&POLLERR)
{
    //打印错误信息
    printf("监听的文件描述符%d发生错误\n",fds[i].fd);
    //定义存储错误信息的数组
    char errors[100];
    memset(errors, '\0', 100);
    socklen_t length=sizeof(errors);
    //获取错误信息
    if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length)<0)
    {
        //获取错误信息失败
        printf("获取socket选项失败。。。\n");
    }
    continue;
}
```



#### 处理客户端关闭事件

```c
//处理远程客户端关闭连接
else if(fds[i].revents&POLLRDHUP)
{
    //将当前关闭的客户端数据用最后一个用户的数据覆盖
    users[fds[i].fd]=users[fds[user_counter].fd];
    close(fds[i].fd);
    fds[i]=fds[user_counter];
    i--;
    user_counter--;
    printf("一个客户端关闭了连接。。。\n");
}
```

- `users[fds[i].fd]=users[fds[user_counter].fd];`将当前关闭的客户端数据用最后一个用户的数据覆盖,这样直接利用最后一个有效元素填充空洞，保持数组紧凑性，同时减少数据移动。
- `fds[i]=fds[user_counter];`同理，避免`fds`数组存在无效元素，保证`poll`只处理有效连接。

#### 处理读事件

这段代码负责处理客户端发送的数据，并根据读取到的数据采取相应的操作。通过监视`POLLIN`事件，服务器能够有效地读取客户端数据，并根据数据的内容通知其他客户端准备发送消息。这确保了服务器能够实时响应客户端的请求，并保持多个客户端之间的通信。

```c
//处理读事件
else if(fds[i].revents&POLLIN)
{
    int connfd=fds[i].fd;
    //清空读缓冲区
    memset(users[connfd].buf, '\0', BUFFER_SIZE);
    //接受数据到读缓冲区
    ret=recv(connfd, users[connfd].buf, BUFFER_SIZE-1, 0);
    printf("接受到来自文件描述符%d的%d字节数据:%s。。。\n",connfd, ret, users[connfd].buf);
    //如果读操作发生错误，则关闭连接
    if(ret<0)
    {
        if(errno!=EAGAIN)
        {
            close(connfd);
            users[fds[i].fd]=users[fds[user_counter].fd];
            fds[i]=fds[user_counter];
            i--;
            user_counter--;
        }
    }
    else if(ret==0)
    {
        //客户端关闭了连接
    }
    else
    {
        //通知其他客户端准备写数据
        for(int j=1;j<=user_counter;++j)
        {
            if(fds[j].fd==connfd)continue;
            //清除客户端上的POLLIN事件，暂时不在监视读事件
            fds[j].events &= ~POLLIN;
            //添加POLLOUT事件，准备写数据
            fds[j].events |= POLLOUT;
            users[fds[j].fd].write_buf=users[connfd].buf;
        }
    }
}
```

- `EAGAIN`表示非阻塞模式下没有数据可读，可以忽略这个错误
- `users[fds[j].fd].write_buf=users[connfd].buf;`将当前客户端的数据复制到其他客户端连接的写缓冲区中，准备发送给其客户端。

#### 处理写事件

```c
//处理写事件
else if(fds[i].revents&POLLOUT)
{
    int connfd=fds[i].fd;
    //如果写缓冲区为空，跳过
    if(!users[connfd].write_buf)
    {continue;}
    //发送数据
    ret=send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
    //清空写缓冲区
    users[connfd].write_buf=NULL;
    //重新注册可读事件
    fds[i].events &= ~POLLOUT;
    fds[i].events |= POLLIN;
}
```

#### 清理回收关闭

```c
//清理回收关闭
free(users);
close(listenfd);
return 0;
```

# 信号

在Linux中信号是消息处理机制的一种，由用户、系统或者进程发送给目标进程的信息，已通知目标进程某个状态的改变或系统异常。

## 信号的状态

- 产生：键盘输入、函数调用、执行shell命令、对硬件进行非法访问等都会产生信号。
- 未决：当信号产生后但还未被处理，此时信号处于未决状态。
- 递达：信号被处理。

## 信号编号

信号的本质其实也是整型数，每个不同的取值代表了不同的信号，在Linux中可以通过shell命令`kill -l`查看系统定义的信号列表：

```
ubuntu16@ubuntu16:~/ubuntu16/project$ kill -l
 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL       5) SIGTRAP
 6) SIGABRT      7) SIGBUS       8) SIGFPE       9) SIGKILL     10) SIGUSR1
11) SIGSEGV     12) SIGUSR2     13) SIGPIPE     14) SIGALRM     15) SIGTERM
16) SIGSTKFLT   17) SIGCHLD     18) SIGCONT     19) SIGSTOP     20) SIGTSTP
21) SIGTTIN     22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO       30) SIGPWR
31) SIGSYS      34) SIGRTMIN    35) SIGRTMIN+1  36) SIGRTMIN+2  37) SIGRTMIN+3
38) SIGRTMIN+4  39) SIGRTMIN+5  40) SIGRTMIN+6  41) SIGRTMIN+7  42) SIGRTMIN+8
43) SIGRTMIN+9  44) SIGRTMIN+10 45) SIGRTMIN+11 46) SIGRTMIN+12 47) SIGRTMIN+13
48) SIGRTMIN+14 49) SIGRTMIN+15 50) SIGRTMAX-14 51) SIGRTMAX-13 52) SIGRTMAX-12
53) SIGRTMAX-11 54) SIGRTMAX-10 55) SIGRTMAX-9  56) SIGRTMAX-8  57) SIGRTMAX-7
58) SIGRTMAX-6  59) SIGRTMAX-5  60) SIGRTMAX-4  61) SIGRTMAX-3  62) SIGRTMAX-2
63) SIGRTMAX-1  64) SIGRTMAX
```

## 信号处理行为

Linux中的man文档中介绍了对产生的信号的五种默认处理动作：

```
Term   Default action is to terminate the process.
Ign    Default action is to ignore the signal.
Core   Default action is to terminate the process and dump core (see core(5)).
Stop   Default action is to stop the process.
Cont   Default action is to continue the process if it is currently stopped.
```

中文说明：

```
Term:进程终止
Ign:忽略信号
Core:进程终止并生成一个core文件(一般用于gdb调试)
Stop:进程暂停
Cont:使暂停的进程继续运行
```

man文档中有提及:

```
The signals SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.
信号SIGKILL(9号信号),信号SIGSTOP(19号)不能被捕捉、阻塞或忽略。
SIGKILL:无条件杀死进程
SIGSTOP:无条件暂停进程
```

## 信号集

在Linux中信号集表示一组信号的集合，数据类型为`sigset_t`，可以存储多个信号的状态。

`sig_set`定义：

```c
#include<bits/sigset.h>
#define _SIGSET_NWORDS (1024/(8*sizeof(unsigned long int)))
typedef struct
{
    unsigned long int __val[_SIGSET_NWORDS];
}__sigset_t;
```

所以`sigset_t`本质是一个无符号长整型数组，其数组大小取决于系统中`unsigned long int`类型的大小。

- 32位系统中，`unsigned long int`通常为32位，即4字节，则信号集数组大小为32个元素。
- 64位系统中，`unsigned long int`通常为64位，即8字节，则信号集数组大小位16个元素。

数组的每个元素的每个位表示一个信号，以64位系统中的`unsigned long int`为例，`sigset_t`内部数组共有16个元素，即数组大小为`16*64=1024`位(128字节)，每一个位都可以记录一个信号的状态。

### 信号集操作函数

```c
#include<signal.h>
//清空信号集，将所有信号的标志位设置为0
int sigemptyset(sigset_t* _set);
//设置所有信号，将所有信号的标志位设置为1
int sigfillset(sigset_t* _set);
//添加信号，将指定的信号(_signo)标志位设置为1
int sigaddset(sigset_t* _set, int _signo);
//删除信号，将指定信号(_signo)标志位设置为0
int sigdelset(sigset_t* _set, int _signo);
//判断是否在信号集内，返回指定信号(_signo)的标志位
int sigismember(const sigset_t* _set, int _signo);
```

### 阻塞信号集

进程控制块PCB中存储的重要信号集之一，用于控制当前进程应当阻塞的信号，当进程接收到一个信号，且此信号在该进程的阻塞信号集中(标志位为1)，则该进程不会立刻处理该信号，暂时保留，直到该信号被移出阻塞信号集(标志位为0)。

> 注意此处的阻塞是指信号延迟处理，并不是丢弃信号，也不是阻止信号的产生。

```c
#include<signal.h>
//阻塞信号集操作函数
int sigprocmask(int _how, const sigset_t* _set, sigset_t* _oset);
```

- `_how`：指定对内核中的阻塞信号集的操作方式，取值如下
  - `SIG_BLOCK`：将`_set`指定的信号集追加到阻塞信号集中
  - `SIG_UNBLOCK`：解除`_set`指定的信号集中的信号的阻塞
  - `SIG_SETMASK`：使用`_set`指定的信号集中的信号覆盖阻塞信号集

- `_set`：指定用于操作的信号集数据
- `_oset`：接受在此次修改之前的阻塞信号集的数据，可以指定为NULL

执行成功返回0，失败返回-1并设置errno

### 未决信号集

进程控制块PCB中存储的重要信号集之一，用于控制当前进程未处理的信号，这些信号已经产生但还未被处理，可能处于阻塞状态也可能进程还没来得及处理。

> 未决状态是指信号已经产生，但还未被处理。

- 如果一个信号被阻塞了，它会处于阻塞信号集中，在此进程接收到此信号之前，该进程的未决信号集中该信号的标志位为0，当此进程接受到该信号时未决信号集才会记录它，即将标志位修改为1。

- 当这个信号的阻塞解除后，该进程将会立即处理该信号，当处理完成后，未决信号集会将其标志位置0。

- 而一个信号如果在被接收后立即处理，那么未决信号集将不会记录它。

```c
#include<signal.h>
int sigpending(sigset_t* set);
```

对于未决信号集，该信号集由内核维护，用户只能进行读操作，获取当前线程的未决信号集。

`set`参数是传出参数，用于接受获取到的内核的未决信号的信号集。

## 相关函数

### 发送信号

#### `kill`

给指定进程发送信号

```c
#include<sys/types.h>
#include<signal.h>
int kill(pid_t pid, int sig);
```

将信号`sig`发送给进程`pid`。`sig`取值即为上面提及的信号编号，`pid`取值如下：

```
pid>0	信号发送给进程PID为pid的进程
pid=0	信号发送给本进程内的其他进程
pid=-1	信号发送给除init进程外的所有进程，但发送者需要拥有对目标发送信号的权限
pid<-1	信号发送给组ID为-pid的进程组中的所有成员
```

信号取值都大于0，如果`sig`参数传入0，则`kill`不发送任何信号。

执行成功返回0，失败返回-1并设置errno，errno取值如下：

```
EINVAL:无效信号
EPERM:该进程无权限发送信号给目标进程
ESRCH:目标进程或进程组不存在
```

使用示例：

```c
//杀死自己
kill(getpid(),9);
//子进程杀死父进程
kill(getppid(),10);
```

### 信号处理

#### `signal`

`signal`系统调用用于指定当捕获到信号时对信号做出何种反应。

```c
#include<signal.h>
_sighandler_t signal(int sig, _sighandler_t _handler);
```

- `sig`：指定要捕获的信号类型。

- `_handler`：指定捕获到信号后要执行的函数，`_sighandler_t`是函数指针类型，原型定义：

  ```c
  #include<signal.h>
  typedef void(*_sighandler_t)(int);
  ```

`signal`中的信号处理函数是由系统内核在捕获到指定类型的信号后调用的，内核会将捕获到的信号编号作为参数传入信号处理函数，以方便信号处理函数可以对不同类型的信号产生不同的行为。

执行成功返回一个函数指针，其类型同样为`_sighandler_t`，是前一次调用`signal`时传入的函数指针，而当此次调用是首次调用时则返回信号`sig`对应的默认处理函数指针`SIG_DEF`。

执行失败返回`SIG_ERR`，设置errno。

#### `sigaction`

`sigaction`功能与`signal`相同，都是捕获信号并对信号做出反应。但是`sigaction`功能更加强大，同样参数更加复杂。

```c
#include<signal.h>
int sigaction(int sig, const struct sigaction* act, struct sigaction* oact);
```

- `sig`：指定要捕获的信号类型。
- `act`：指定对捕获到的信号的处理方式。
- `oact`：上一次函数调用对该信号的处理方式，一般指定为NULL。

其中`sigaction`结构体类型定义如下：

```c
struct sigaction
{
    #ifdef __USE_POSIX 199309
    union
    {
        _sighandler_t sa_handler;
        void (*sa_sigaction)(int, siginfo_t*, void*);
    }
    _sigaction_handler;
    #define sa_handler __sigaction_handler.sa_handler
    #define sa_sigaction __sigaction_handler.sa_sigaction
    #else
    _sighandler_t sa_handler;
    #endif
    _sigset_t sa_mask;
    int as_flags;
    void (*sa_restorer)(void);
};
```

- `sa_handler`：函数指针，指定信号处理函数。

- `sa_sigaction`：函数指针，指定信号处理函数。

- `sa_mask`：信号集`sigset_t`类型，设置进程的信号掩码，以在信号处理函数执行期间临时屏蔽一些信号，将要屏蔽的信号添加进信号集即可。当处理函数执行完毕后，临时信号屏蔽会自动解除，同时此信号集至少有一个信号，即捕获的信号。

- `sa_flags`：指定捕获到信号后的处理行为，其可选值如下：

  ```
  0：使用sa_handler，默认
  SA_SIGINFO：使用sa_sigaction作为信号处理函数而不是默认的sa_handler
  SA_INTERRUPT：中断系统调用
  ```

- `sa_restorer`：已废弃，不要使用。

函数执行成功返回0，失败返回-1并设置errno。
