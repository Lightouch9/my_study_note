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

# 进程控制

进程控制包括结束进程、回收资源，同时涉及进程的特殊状态：孤儿进程和僵尸进程。

## 进程结束

在程序中可以通过`exit()`或`_exit()`结束进程。

```c
#include<stdlib.h>
//标准c库函数
void exit(int status);
//Linux系统函数
void _exit(int status);
```

参数`status`是退出码，指定进程退出的状态码。

也可以在`main`函数中通过`return`退出进程。

```c
int main()
{
    /*代码*/
    return 0;	//进程退出
}
```

## 孤儿进程

在一个进程中创建一个子进程，父子进程同时运行，但是由于某些原因父进程先于子进程退出结束了，此时这个正在运行的子进程就是孤儿进程。

此时系统检测到这个孤儿进程，会将这个孤儿进程的`PPID`设置为1，即`init`进程，由这个进程领养这个孤儿进程。由`init`进程回收子进程退出结束后的资源。

## 僵尸进程

在多进程程序中，一般是由父进程负责子进程的资源释放回收，但是当由于某些原因在子进程结束运行后，父进程无法或没有正确处理子进程的返回信息，没能正确释放子进程对于内核资源的占用，这时这个子进程就成为了僵尸进程。

此时子进程虽然已经结束已经死亡，但是在内核中的进程表的表项没有释放，依然占用着内核资源，这样会产生资源浪费。

想要消灭僵尸进程，则需要杀死僵尸进程的父进程，回收僵尸进程的资源。

## 进程回收

为了避免僵尸进程的产生，需要确保父进程在子进程结束推出后会回收子进程的资源。

### wait()

```c
#include<sys/wait.h>
pid_t wait(int* stat_loc);
```

`wait`是阻塞函数，它会阻塞当前进程，直到某个子进程结束运行。

`stat_loc`：传出参数，用于记录子进程的退出信息，可以指定为NULL，解析这些退出信息需要使用一些宏函数：

- `WIFEXITED(stat_loc)`：返回1，进程正常退出
- `WEXITSTATUS(stat_loc)`：如果`WIFEXITED`非0，则返回子进程的退出码
- `WIFSIGNALED(stat_loc)`：返回1，则子进程因为一个未捕获的信号终止
- `WTERMSIG(stat_loc)`：如果`WIFSIGNALED`非0，则返回导致子进程终止的信号值

执行成功返回被回收的子进程的进程id，执行失败则返回-1，失败一般是因为没有子进程可以回收或者回收资源时发生了异常。

### waitpid()

如果不希望阻塞式回收子进程资源，可以使用`waitpid()`，该函数可以控制回收子进程资源方式是阻塞或非阻塞。

```c
#include<sys/wait.h>
pid_t waitpid(pid_t pid, int* stat_loc, int options);
```

`pid`：指定回收对象，取值如下：

- -1：回收任意一个子进程，同`wait()`
- 大于0：指定回收的子进程id
- 0：回收当前进程组的所有子进程id
- 小于-1：`pid`的绝对值指定进程组id，表示要回收这个进程组的所有子进程资源

`stat_loc`：同`wait()`的`stat_loc`，接受子进程的退出信息

`options`：控制阻塞还是非阻塞，取值如下：

- `0`：阻塞
- `WNOHANG`：非阻塞

返回值：

- 非阻塞：
  - 如果`pid`指定的子进程没有结束或终止，则立即返回0回收成功

- 回收成功，返回子进程的进程id
- 执行失败，返回-1，同`wait()`。

通过`waitpid()`非阻塞式回收子进程资源时，最好在子进程退出后再回收，可以通过`SIGCHLD`信号来判断。

> 当子进程退出、暂停或者从暂停中回复运行时，子进程会产生一个`SIGCHLD`信号发送给父进程，默认情况下父进程会忽略此信号，但是可以在父进程中捕获此信号来回收子进程的资源。

```c
//父进程中对于SIGCHLD信号的处理函数
static void handle(int sig)
{
    pid_t pid;
    int stat;
    while((pid=waitpid(-1, &stat, WNOHANG))>0)
    {
		/*对子进程善后处理*/
    }
}
```

# 信号量

## semget

该函数用于创建或访问一组信号量，即信号量集。

```c
#include<sys/sem.h>
int semget(key_t key, int num_sems, int sem_flags);
```

- `key`：信号量集键值，用于全局唯一的标识一个信号量集。
- `nums_sems`：指定信号量集中信号量的数量，如果是创建信号量集则必须指定，如果是创建则可指定为0。
- `sem_flags`：指定创建或获取信号量集操作的标志，取值可以是多个值的按位或，其格式与含义与系统调用中文件I\O的`open`的`mode`参数相同。
  - `IPC_CREAT`：如果系统中不存在`key`为键值的信号量集，则创建一个新的信号量集，如果已经存在此键值，则忽略此标志。
  - `IPC_EXCL`：与`IPC_CREAT`一起使用，即`IPC_CREAT |IPC_EXCL`，如果`key`指定的信号集已存在，则调用失败，并返回错误设置errno为`EEXIST`，可以确保创建的信号量集是唯一的。
  - 权限位：如0666，设置信号量集的权限，这些权限与文件权限类似，但解释不同

​	执行成功返回创建的信号量集的键值，失败返回-1.并设置errno。

信号量集的数据类型为`semid_ds`，其定义如下：

```c
#include<sys/sem.h>
//描述ipc对象的权限信息
struct ipc_perm
{
    //创建信号量集时提供的键值
    key_t key;
    //所有者有效用户id
    uid_t uid;
    //所有者有效用户组id
    gid_t gid;
    //创建者有效用户id
    uid_t cuid;
    //创建者有效用户组id
    gid_t cgid;
    //权限模式
    mode_t mode;
}
struct semid_ds
{
    //信号量集的操作权限和所有者信息
    struct ipc_perm sem_perm;
    //信号量集中信号量的数目
    unsigned long int sem_nsems;
    //最后一次调用semop的时间
    time_t sem_otime;
    //最后一次调用semctl的时间
    time_t sem_ctime;
}
```

`semget()`对信号集`semid_ds`的初始化操作如下：

- 将`sem_perm.uid`所有者id和`sem_perm.cuid`创建者id设置为调用该进程的用户id
- 将`sem_perm.gid`所有者用户组id和`sem_perm.cgid`创建者用户组id设置为调用进程的组id
- 将`sem_perm.mode`的低9位设置为`sem_flags`的低9位
- 将`sem_nsems`设置为`num_sems`
- 将`sem_otime`设置位0
- 将`sem_ctime`设置为当前的系统时间

## semop

`semop`用于操作信号量，即P、V操作，用于控制对临界资源的访问。

首先是与`semop`相关联的内核中与信号量有关的变量：

```c
unsigned short semval;	//信号量的值
unsigned short semzcnt;	//等待信号量的值变为0的进程的数量
unsigned short semncnt;	//等待信号量的值增加的进程数量
pid_t sempid;	//最后一次执行semop的进程id
```



```c
#include<sys/sem.h>
int semop(int sem_id, struct sembuf* sem_ops, size_t num_sem_ops);
```

- `sem_id`是通过`semget`调用返回的信号量集标识符，用于指定被操作的信号量集。

- `sem_ops`指向一个`sembuf`类型的数组，用于指定对信号量的操作

  ```c
  struct sembuf
  {
      unsigned short int sem_num;	//信号量集中信号量的编号，从0开始
      short int sem_op;	//执行的操作，增、减、等待
      short int sem_flg;	//操作标志，SEM_UNDO、IPC_NOWAIT
  }
  ```

  - `sem_op`指定操作类型，一般与`sem_flg`操作标志共同控制对于信号量的操作

    - `sem_op`大于0，则`semop`将被操作的信号量的值`semval`增加`sem_op`，该操作需要调用进程拥有对于被操作信号量的写权限。

      如果此时`sem_flg`为`SEM_UNDO`，则系统会更新进程的`semadj`变量(用于跟踪进程对于信号量的修改情况)

    - `sem_op`等于0，则表示是等待操作，该操作要求调用进程拥有对于被操作信号量的读权限，如果信号量的值`semval`为0，则调用成功立即返回。

      如果`semval`不为0，则

      - 如果`sem_flg`为`IPC_NOWAIT`，即指定非阻塞，`semop`会立即返回一个错误，并设置errno为`EAGAIN`。
      - 如果未指定`IPC_NOWAIT`，则`semzcnt`值加1，进程进入休眠，唤醒条件为：
        - `semval`变为0，然后`semzcnt`减1
        - 所操作的信号量所在的信号量集被移除，`semop`调用失败返回，设置errno为`EIDRM`
        - 此次调用被信号中断，`semop`调用失败返回，设置errno为`EINTR`，同时`semzcnt`减1

    - `sem_op`小于0，则进行减操作，即进行信号量的获取，需要拥有对于信号量的写权限

      - `semval`大于`sem_op`的绝对值，则`semop`操作成功，将`semval`减去`sem_op`的绝对值，获取到了信号量。

        如果`sem_flg`为`SEM_UNDO`，则系统将更新进程的`semadj`变量

      - `semval`小于`sem_op`的绝对值，则

        - `sem_flg`为`IPC_NOWAIT`，则立即返回一个错误，设置errno为`EAGAIN`

        - `sem_flg`不为`IPC_WAIT`，则`semncnt`加1，进程进入休眠，唤醒条件为：

          - `sem_val`大于等于`sem_op`的绝对值，则`semncnt`减1，`semval`减去`sem_op`的绝对值。

            如果`SEM_UNDO`被设置，则系统更新进程的`semadj`变量。

          - 所操作的信号量所在的信号量集被移除，`semop`调用失败返回，设置errno为`EIDRM`

          - 此次调用被信号中断，`semop`调用失败返回，设置errno为`EINTR`，同时`semncnt`减1

- 参数`num_sem_ops`指定要执行的操作的个数，即要执行`sem_ops`数组的元素的个数，并且执行顺序按照数组顺序依次执行，且为原子操作。

执行成功返回0，失败返回-1并设置errno，如果失败，`sem_ops`数组的所有操作都不会执行。

## semctl

`semctl`用于对信号量进行直接控制，如对信号量集中的信号进行查询、设置、删除等操作。

```c
#include<sys/sem.h>
int semctl(int sem_id, int sem_num, int command, ...);
```

- `sem_id`：由`semget`返回的信号量集标识符，用于指定要操作的信号量集

- `sem_num`：指定要操作的信号量在信号量集中的索引。

- `command`：指定要执行的命令，该命令可能会用到第4个参数

- `...`：补充`command`可能用到的参数，虽然此参数可以由用户定义，但头文件`sys/sem.h`中给出了推荐数据类型：

  ```c
  #include<sys/sem.h>
  union semun
  {
      int val;	//用于SETVAL命令
      struct semid_ds* buf;	//用于IPC_STAT和IPC_SET命令
      unsigned short* array;	//用于GETALL和SETALL命令
      struct seminfo* __buf;	//用于IPC_INFO命令
  };
  
  struct seminfo
  {
      int semmap;
      int semmni;
      int semmns;
      int semmsl;
      int semopm;
      int semume;
      int semusz;
      int semvmx;
      int semaem;
  };
  ```

  

`semctl`可以执行的命令，即`command`可取值如下：

|    命令    |                             含义                             |  成功调用返回值   |
| :--------: | :----------------------------------------------------------: | :---------------: |
| `IPC_STAT` |       获取信号量集的状态信息，将其复制到`semun.buf`中        |         0         |
| `IPC_SET`  | 设置信号量集的状态信息，使用用户提供的`semid_ds`更新内核中的信号量集数据结构，并更新内核的`semid_ds.sem_ctime` |         0         |
| `IPC_RMID` |         立即移除信号量集，并唤醒等待该信号量集的进程         |         0         |
|  `GETVAL`  |     获取信号量集中指定的信号量的值，保存至`semun.array`      |         0         |
|  `SETVAL`  | 设置信号量集中指定信号的值为`semun.val`，并更新内核中的`semid_ds.sem_ctime` |         0         |
|  `GETPID`  |                   获取指定信号量的`sempid`                   | 信号量的`sempid`  |
| `GETNCNT`  |                    获取信号量的`semncnt`                     | 信号量的`semncnt` |
| `GETZCNT`  |                    获取信号量的`semzcnt`                     | 信号量的`semzcnt` |
|            |                                                              |                   |
|            |                                                              |                   |
|            |                                                              |                   |
|            |                                                              |                   |
|            |                                                              |                   |

`GETNCNT`、`GETPID`、`GETVAL`、`GETZCNT`、`SETVAL`操作的是单个信号量，其余的是操作的整个信号量集，此时`sem_num`参数会被忽略。

执行成功返回值取决于`command`参数，失败返回-1，并设置errno。

# 共享内存

共享内存是所有进程间通信的方式中效率最高的，共享内存不属于任何进程，也不受与其关联的进程的生命周期影响，不涉及进程之间的任何数据传输。

但是因为共享内存操作默认是不阻塞的，所以需要其他手段同步多个进程对于共享内存的访问，否则容易产生竞态条件，造成数据混乱。

接下来介绍有关共享内存的4个系统调用：

## `shmget`

该系统调用用于创建一段新的共享内存，或者打开一段已经存在的共享内存。

```c
#include<sys/shm.h>
int shmget(key_t key, size_t size, int shmflg);
```

- `key`：同`semget`，该参数是一个键值，标识唯一一段共享内存，用于指定创建或打开的共享内存的键值。
- `size`：仅在创建共享内存时生效，用于指定创建的共享内存的大小，单位为字节，打开共享内存时可以指定为0。
- `shmflg`：创建共享内存时指定的属性，取值同`semget`的`sem_flags`参数，但额外支持两个值，`SHM_HUGETLB`和`SHM_NORESERVE`。
  - `SHM_HUGETLB`：系统将使用“大页面”来为共享内存分配空间。
  - `SHM_NORESERVE`：不为共享内存保留交换分区(swap分区)，当物理内存空间不足时对该共享内存执行写操作将触发`SIGSEGV`信号。

执行成功返回共享内存的标识符，失败返回-1，指定errno。

该调用创建的共享内存内部所有字节都将初始化为0，相关联的内核数据结构`shmid_ds`将被创建，用于管理每段共享内存的信息。

```c
struct shmid_ds
{
    struct ipc_perm shm_perm;	//共享内存的操作权限信息
    size_t shm_segsz;	//共享内存大小，单位为字节
    __time_t shm_atime;	//对此共享内存最后一次调用shmat的时间
    __time_t shm_dtime;	//对此共享内存最后一次调用shmdt的时间
    __time_t shm_ctime;	//对此共享内存最后一次调用shmctl的时间
    __pid_t shm_cpid;	//创建该共享内存的进程的id
    __pid_t shm_lpid;	//对该共享内存最后一次执行shmat或shmdt的进程的id
    shmatt_t shm_nattach;	//当前关联到此共享内存的进程的数量
};
```

`shmget`对`shmid_ds`的初始化操作如下：

- `shm_perm.cuid`和`shm_perm.uid`设置为调用进程的用户id
- 将`shm_perm.cgid`和`shm_perm.gid`设置为调用进程的组id
- 将`shm_perm.mode`的低9位设置位`shmflg`参数的低9位
- 将`shm_segsz`设置为`size`
- 将`shm_lpid`、`shm_nattach`、`shm_atime`、`shm_dtime`设置为0
- 将`shm_ctime`设置为当前时间

## `shmat`

该调用用于将创建或者打开的共享内存关联到当前进程的地址空间中，这样才能得到共享内存的起始地址，进行数据的读写。

```c
#include<sys/shm.h>
void* shmat(int shm_id, const void* shm_addr, int shmflg);
```

- `shm_id`：要关联的共享内存标识符
- `shm_addr`：指定将共享内存关联到进程的哪块地址空间，同时也会受`shmflg`参数的影响，一般设置为NULL，此时地址由内核决定(推荐)
- `shmflg`：标志位：
  - `SHM_RDONLY`：进程仅可只读共享内存
  - `SHM_REMAP`：如果地址`shmaddr`已经被关联到一段共享内存，则重新关联
  - 0：读写权限，可读可写
  - 如果`shm_addr`非空，且未设置`SHM_RND`标志位，则共享内存被关联到`shm_addr`指定的地址

执行成功返回被关联到的地址，失败返回`(void*)-1`并设置errno

执行成功时内核中的`shmid_ds`中的部分字段会被修改：

- `shm_nattach`加1
- `shm_lpid`设置为调用进程的进程id
- `shm_atime`设置为当前时间

## `shmdt`

该调用用于将关联的共享内存解除关联。

```c
#include<sys/shm.h>
void* shmat(const void* shm_addr);
```

`shm_addr`是`shmat()`的返回值，共享内存的起始地址。

执行成功返回0，失败返回-1并设置errno

执行成功会修改内核中的`shmid_ds`的部分字段：

- `shm_nattach`减1
- `shm_lpid`设置为调用进程的id
- `shm_dtime`设置为当前时间

## `shmctl`

该调用用于控制共享内存的一些属性，可以设置、获取共享内存的状态。

```c
#include<sys/shm.h>
int shmctl(int shm_id, int command, struct shmid_ds* buf);
```

- `shm_id`：共享内存标识符
- `command`：指定要执行的命令

|    命令    |                     含义                      | 成功返回值 |
| :--------: | :-------------------------------------------: | :--------: |
| `IPC_STAT` |   获取当前共享内存的状态信息，复制到`buf`中   |     0      |
| `IPC_SET`  | 设置共享内存的状态，将`buf`的数据设置到内核中 |     0      |
| `IPC_RMID` |           将共享内存设置为删除状态            |     0      |
|            |                                               |            |
|            |                                               |            |
|            |                                               |            |
|            |                                               |            |
|            |                                               |            |

删除状态并不是立即删除，而是当所有关联到此共享内存的进程断开关联后才删除。

执行成功返回值取决于`command`执行的命令，失败返回-1并设置errno
