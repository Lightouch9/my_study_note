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