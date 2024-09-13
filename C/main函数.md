# main()参数解释

main()函数一般是一个c/c++程序的执行入口，通常情况下main()在编写时并不会设定参数，但其实main()是有参数的。

```c
int main(int argc, char* argv[]);
```

在c++中第二个参数有时会使用`char** argv`。

1. `int argc`:`argc`代表`argument count`，即参数的数量，表示通过命令行传递给程序的命令行参数的数量，这个参数的值至少为1，第一个参数`argv[0]`总是程序的名称或者路径。
2. `char* argv[]`:`argv`意为`argument vector`，即参数向量或者参数数组，这个字符指针数组的每个元素都是一个指向一个字符串的指针，这个字符串就是传入的命令行参数，并且字符串以结束符`\0`结尾。通常这个数组的第一个元素`argv[0]`是程序的名称或路径。同时`argv`的最后一个元素为`NULL`，即`argv[argc]`为`NULL`，所以注意索引不要越界。

```c
#include<stdio.h>

int main(int argc, char* argv[])
{
    printf("main函数接收到的命令行参数数量：%d\n",argc);
    for(int i=0;i<argc;i++)
    {
        printf("main函数接收到的第%d个命令行参数：%s\n",i+1,argv[i]);
    }
    return 0;
}
```

程序测试结果：

```
ubuntu16@ubuntu16:~/ubuntu16/project/c程序测试$ ./main_test
main函数接收到的命令行参数数量：1
main函数接收到的第1个命令行参数：./main_test

ubuntu16@ubuntu16:~/ubuntu16/project/c程序测试$ ./main_test 你好
main函数接收到的命令行参数数量：2
main函数接收到的第1个命令行参数：./main_test
main函数接收到的第2个命令行参数：你好
```

通过这两个`main()`的参数可以灵活地通过命令行参数控制程序的执行。