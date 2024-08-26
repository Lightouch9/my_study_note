# 工作流程
守护进程syslogd，升级版rsyslogd
![[Pasted image 20240826204945.png]]

# syslog
应用程序通过syslog函数与rsyslogd守护进程通信
```c
#include<syslog.h>
void syslog(int priority, const char* message);
```
openlog可以改变syslog的默认输出方式
```c
#include<syslog.h>
void openlog(const char* ident, int logopt, int facility);
```
设置syslog日志掩码
```c
#include<syslog.h>
int setlogmask(int maskpri);
```
关闭日志功能
```c
#include<syslog.h>
void closelog();
```