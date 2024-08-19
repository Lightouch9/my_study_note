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

主机字节序转网络字节序：

```c
int inet_pton(int af, const char *src, void *dst);
```

网络字节序转主机字节序：

```c
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
```

# 通用Socket地址
## sockaddr
sockaddr是一种结构体，用于表示socket地址，数据结构定义：
```c
#include＜bits/socket.h＞ 
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

# 通信流程