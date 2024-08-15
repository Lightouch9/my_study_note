Linux系统提供的文件I/O操作用的函数，这些函数只适用于Linux系统。
# 文件打开关闭
## open
open函数用于打开一个磁盘文件，并且可以指定在不同情况下的打开操作。
```c
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
//打开已存在的文件
int open(const char *pathname, int flags);
//打开的文件不存在，自动创建
int open(const char *pathname, int flags, mode_t mode);
```
参数介绍：
- pathname: 指定要打开的文件的路径，可以使用绝对路径或相对路径。
- flags: 指定文件打开方式，有以下三种属性：
	- `O_RDONLY`: 只读
	- `O_WRONLY`: 只写
	- `O_RDWR`: 读写
	- 同时还有一些额外属性与上面3个属性配合使用：
		- `O_APPEND`: 新数据会追加到文件尾部，而不是覆盖现有的内容
		- `O_CREATE`: 如果打开的文件不存在则会创建该文件，如果已经存在则什么都不做
		- `O_EXCL`: 检测文件是否存在，需要搭配`O_CREATE`一起使用：`O_CREATE | O_EXCL`
			- 如果检测到文件存在，则创建失败，返回-1（不添加此属性则不返回-1）。
			- 如果检测到文件不存在，则创建新文件。
- mode: 创建新文件时需要指定的参数，用于指定新文件的权限，数据类型为8进制整数
	- 最大值为0777
返回值：
- 成功执行：返回打开的文件的文件描述符，一个大于0的整数
- 失败：返回-1
## close
close用于关闭打开的文件，释放分配给所打开的文件的文件描述符。
```c
#include<unistd.h>
int close(int fd);
```
参数介绍：
- fd: 需要释放的文件描述符，即前面open()的返回值；
返回值：
- 成功：返回0；
- 失败：返回-1。