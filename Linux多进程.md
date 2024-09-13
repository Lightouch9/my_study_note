# fork

```c
#include<sys/types.h>
#include<unisted.h>
pid_t fork(void);
```

在一个进程中调用`fork()`就会得到一个新的进程，这个新的进程一般称为子进程，调用`fork()`的进程就是主进程或父进程。

`fork()`调用成功后会返回两个值，在父进程中返回子进程的`pid`，在子进程中返回0。于是可以通过返回值判断当前进程是父进程还是子进程，以执行不同的代码。

调用失败返回-1，并设置errno。

## 子进程地址空间的复制

每个进程都有自己的虚拟地址空间，`fork()`得到的子进程的虚拟地址空间是基于父进程的地址空间复制的，但是存储的信息并不完全相同。

子进程的用户区数据与父进程的相同，包括：

代码区、全局数据区、堆区、动态库加载区、栈区、环境变量、文件描述符表。

不同之处在于：

- 父子进程的虚拟地址空间是相互独立，互不影响的。
- 父子进程的进程id`pid`是不同的。
- 子进程的`PPID`会设置为父进程的`pid`。
- 子进程的信号位图(记录信号状态)也会被清空，原进程设置的信号处理函数不对新的进程生效。
- 子进程是一个独立的进程，需要单独竞争cpu时间，因此子进程的运行状态可能与父进程不同。
- `fork()`调用的返回值会在父进程的地址空间中返回子进程的`pid`，在子进程的地址空间中返回0，因此可以通过返回值的不同执行不同的代码。

子进程对于父进程地址空间数据的复制是**写时复制**，`copy on write`，只有父进程或者子进程对于数据进行写操作时才会进行真正的复制。

子进程的代码区与父进程相同，但是由于子进程是在`fork()`之后才产生的，所以子进程会从`fork()`之后继续执行代码。

## 代码示例

```c
#include<stdio.h>
#include<unistd.h>

int main()
{
    //创建子进程
    pid_t pid=fork();
    printf("此进程的返回值是：%d\n",pid);
    //父进程执行代码
    if(pid>0)
    {
        printf("这里是父进程，进程id是%d\n",getpid());
    }
    //子进程执行代码
    else if(pid==0)
    {
        printf("这里是子进程，进程id是%d\n",getpid());
    }
    else
    {
        printf("进程创建失败。。。\n");
    }
    //父、子进程都执行的代码
    printf("fork test\n");

    return 0;
}
```

执行结果：

```
此进程的返回值是：2864
这里是父进程，进程id是2863
fork test
此进程的返回值是：0
这里是子进程，进程id是2864
fork test
```

# exec系列系统调用

## 原型定义

`exec`系列系统调用用于在子进程执行其他程序，或通过子进程启动磁盘的上的另一个可执行程序。`exec`系列系统调用如下：

```c
#include<unistd.h>
extern char** environ;
int execl(const char* path, const char* arg, ...);
int execlp(const char* file, const char* arg, ...);
int execle(const char* path, const char* arg, ..., char* const envp[]);
int execv(const char* path, char* const argv[]);
int execvp(const char* file, char* const argv[]);
int execve(const char* path, char* const argv[], char* const envp[]);
```

> `const char*`与`char* const`的区别
>
> `const char*`是指这个指向`char`类型变量的指针指向的字符内容不可修改，但是这个指针本身可以被修改，使其指向其他字符。
>
> `char* const`是指这个指向`char`类型变量的指针不可修改，这个指针是常量，它不可指向其他字符，但是其指向的字符可以被修改。

## 参数说明

- `path`：指定可执行文件的完整路径
- `file`：指定文件名，文件具体的位置会在环境变量`PATH`中寻找
- `arg`：接受可变参数，传递给新程序的`main`函数，这个参数一般用于指定新进程的名字，一般与指定的可执行程序的名称相同
- `...`：执行可执行程序需要的命令行参数，可以指定多个，与在命令行调用相同，以NULL结尾，表示参数指定完毕。
- `argv`：接受参数数组，传递给新程序的`main`函数，相当于将`arg`参数与`...`封装到一起，该数组的第一个元素为新进程的名字，同`arg`参数，其余元素同`...`参数。
- `envp`：设置新程序的环境变量，未指定则使用全局变量`environ`指定的环境变量。

## 注意要点

- `exec`系列调用执行成功一般没有返回值，因为执行成功后进程的虚拟地址空间已经完全被新的内容替换，包括代码段、数据段、堆栈等，只有执行失败时才会返回-1并设置errno。
- `exec`函数一般不会关闭原程序打开的文件描述符，除非该文件描述符设置了类似`SOCK_CLOEXEC`的选项设置。

- 注意，`exec`系列调用无法创建新的进程，只能**替换**进程，将进程的虚拟地址空间的内容进行替换。

## 常用调用

`exec`系列系统调用最常用的有`execl()`与`execlp()`，这两个函数是对其他4个函数的进一步封装。

### execl()

```c
#include<unistd.h>
int execl(const char* path, const char* arg, ...);
```

该函数用于执行任意一个可执行程序，可执行程序的路径通过`path`参数指定。

执行成功没有返回值，执行失败返回-1。

### execlp()

```c
#include<unistd.h>
int execlp(const char* file, const char* arg, ...);
```

该函数用于执行已经设置了环境变量的可执行程序，函数名中的`p`是指`PATH`，这个函数会从环境变量`PATH`中搜寻指定的文件名，所以可执行程序无需指定完整路径，当然前提是坏境变量已经设置好了。

执行成功没有返回值，执行失败返回-1。

## 代码示例

由于`exec`调用会将当前进程的虚拟地址空间覆盖，所以一般都是先`fork()`创建一个新的子进程，在子进程中调用`exec`调用，这样就不会影响父进程的代码执行。

```c
#include <stdio.h>
#include <unistd.h>

int main()
{
    //创建子进程
    pid_t pid = fork();
    //在子进程中执行磁盘上的可执行程序
    if(pid == 0)
    {
        // 磁盘上的可执行程序 /bin/ps
        //execl()
        execl("/bin/ps", "title", "aux", NULL);
        // 也可以这么写
        // execl("/bin/ps", "title", "a", "u", "x", NULL);  
        
        
        //execlp()
        //execlp("ps", "title", "aux", NULL);
        // 也可以这么写
        // execl("ps", "title", "a", "u", "x", NULL);

        // 如果成功当前子进程的代码区别 ps中的代码区代码替换
        // 下面的所有代码都不会执行
        // 如果函数调用失败了,才会继续执行下面的代码
        perror("execl");
        printf("++++++++++++++++++++++++\n");
        printf("++++++++++++++++++++++++\n");
    }
    else if(pid > 0)
    {
        printf("我是父进程.....\n");
    }
    return 0;
}
```

执行结果(部分)：

```
ubuntu16   3953  0.0  0.1  44432  3344 pts/11   R    23:31   0:00 title aux
```

