# Linux
## EXT2/EXT3/EXT4文件系统文件的存取
### 文件的创建
> 如果是新建一个文件或目录则需要利用 block bitmap(存储未使用的inode号码) 和 inode bitmap(存储未使用的block号码)，大概过程为：
```
1.先确定使用者对要新建文件的目录是否拥有w(写入)与x(执行)的权限；
2.根据inode bitmap查找未使用的inode号码，并将要创建的新文件的权限与属性写入；
3.根据block bitmap查找未使用的block号码，并将要写入的数据写入block 中，并将上一步的inode指向该block;
4.将新创建的inode与block数据同步更新到inode bitmap与block bitmap中的数据，并更新superblock的内容。
```

## 进程管理

### 进程的概念

>  进程(process)是系统内一个正在运行的程序的实例，是系统分配资源的基本单位，这些资源为cpu时间、内存、磁盘空间、I/O设备、寄存器、文件描述符等，每个进程都有唯一的标识符(PID)以及独立的内存空间和系统资源视图，但同时进程之间都共享相同的物理内存与硬件资源。
>  系统中的每一个程序都运行在某个上下文中。
```
上下文是对程序在系统中运行时其执行环境与状态信息的概述，包括此程序存储在内存中的代码和数据，比如栈、通用目的寄存器的内容、程序计数器、环境变量、打开的文件描述符等。
```
> 在Linux中，进程的创建与调度由系统内核负责，内核通过调度器决定哪些进程可以运行，并根据优先级和调度策略进行调度。
### 进程控制块(PCB)
#### 定义
> linux系统用于描述控制进程的一个专门的数据结构，用于记录进程的各项信息，是系统感知进程存在的唯一标志，与进程之间是一一对应的关系，该数据结构在Linux内的具体实现是`task_struct`，也被称为进程描述符。
#### task_struct
> `task_struct`进程描述符是Linux内核中用于描述进程数据结构，它包含了一个进程的所有数据信息，能够完整地描述一个运行的程序。
```
- 进程id/标识符：描述本进程的唯一标识符，用于区别其他进程，类型为pid_t，本质是一个整型数。
- 状态：表示任务状态、退出代码、退出信号等。
- 优先级：相对于其他进程的优先级。
- 程序计数器：程序中即将被执行的下一条指令的地址。
- 内存指针：包括程序代码和进程相关数据的指针，以及与其他进程共享的内存块的指针。
- 上下文数据：进程执行时处理器的寄存器中的数据。
- I/O状态信息：包括显示的I/O请求、分配给进程的I/O设备和被进程使用的文件列表。
- 记账信息：可能包括处理器的使用情况、时间限制、记账号等。
```
#### task_struct源码
> 以下源码来自/include/linux/sched.h
```c
struct task_struct {
	volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	void *stack;
	atomic_t usage;
	unsigned int flags;	/* per process flags, defined below */
	unsigned int ptrace;
 
	int lock_depth;		/* BKL lock depth */
 
#ifdef CONFIG_SMP
#ifdef __ARCH_WANT_UNLOCKED_CTXSW
	int oncpu;
#endif
#endif
	int load_weight;	/* for niceness load balancing purposes */
	int prio, static_prio, normal_prio;
	struct list_head run_list;
	struct prio_array *array;
 
	unsigned short ioprio;
#ifdef CONFIG_BLK_DEV_IO_TRACE
	unsigned int btrace_seq;
#endif
	unsigned long sleep_avg;
	unsigned long long timestamp, last_ran;
	unsigned long long sched_time; /* sched_clock time spent running */
	enum sleep_type sleep_type;
 
	unsigned int policy;
	cpumask_t cpus_allowed;
	unsigned int time_slice, first_time_slice;
 
#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	struct sched_info sched_info;
#endif
 
	struct list_head tasks;
	/*
	 * ptrace_list/ptrace_children forms the list of my children
	 * that were stolen by a ptracer.
	 */
	struct list_head ptrace_children;
	struct list_head ptrace_list;
 
	struct mm_struct *mm, *active_mm;
 
/* task state */
	struct linux_binfmt *binfmt;
	int exit_state;
	int exit_code, exit_signal;
	int pdeath_signal;  /*  The signal sent when the parent dies  */
	/* ??? */
	unsigned int personality;
	unsigned did_exec:1;
	pid_t pid;
	pid_t tgid;
 
#ifdef CONFIG_CC_STACKPROTECTOR
	/* Canary value for the -fstack-protector gcc feature */
	unsigned long stack_canary;
#endif
	/* 
	 * pointers to (original) parent process, youngest child, younger sibling,
	 * older sibling, respectively.  (p->father can be replaced with 
	 * p->parent->pid)
	 */
	struct task_struct *real_parent; /* real parent process (when being debugged) */
	struct task_struct *parent;	/* parent process */
	/*
	 * children/sibling forms the list of my children plus the
	 * tasks I'm ptracing.
	 */
	struct list_head children;	/* list of my children */
	struct list_head sibling;	/* linkage in my parent's children list */
	struct task_struct *group_leader;	/* threadgroup leader */
 
	/* PID/PID hash table linkage. */
	struct pid_link pids[PIDTYPE_MAX];
	struct list_head thread_group;
 
	struct completion *vfork_done;		/* for vfork() */
	int __user *set_child_tid;		/* CLONE_CHILD_SETTID */
	int __user *clear_child_tid;		/* CLONE_CHILD_CLEARTID */
 
	unsigned int rt_priority;
	cputime_t utime, stime;
	unsigned long nvcsw, nivcsw; /* context switch counts */
	struct timespec start_time;
/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
	unsigned long min_flt, maj_flt;
 
  	cputime_t it_prof_expires, it_virt_expires;
	unsigned long long it_sched_expires;
	struct list_head cpu_timers[3];
 
/* process credentials */
	uid_t uid,euid,suid,fsuid;
	gid_t gid,egid,sgid,fsgid;
	struct group_info *group_info;
	kernel_cap_t   cap_effective, cap_inheritable, cap_permitted;
	unsigned keep_capabilities:1;
	struct user_struct *user;
#ifdef CONFIG_KEYS
	struct key *request_key_auth;	/* assumed request_key authority */
	struct key *thread_keyring;	/* keyring private to this thread */
	unsigned char jit_keyring;	/* default keyring to attach requested keys to */
#endif
	/*
	 * fpu_counter contains the number of consecutive context switches
	 * that the FPU is used. If this is over a threshold, the lazy fpu
	 * saving becomes unlazy to save the trap. This is an unsigned char
	 * so that after 256 times the counter wraps and the behavior turns
	 * lazy again; this to deal with bursty apps that only use FPU for
	 * a short time
	 */
	unsigned char fpu_counter;
	int oomkilladj; /* OOM kill score adjustment (bit shift). */
	char comm[TASK_COMM_LEN]; /* executable name excluding path
				     - access with [gs]et_task_comm (which lock
				       it with task_lock())
				     - initialized normally by flush_old_exec */
/* file system info */
	int link_count, total_link_count;
#ifdef CONFIG_SYSVIPC
/* ipc stuff */
	struct sysv_sem sysvsem;
#endif
/* CPU-specific state of this task */
	struct thread_struct thread;
/* filesystem information */
	struct fs_struct *fs;
/* open file information */
	struct files_struct *files;
/* namespaces */
	struct nsproxy *nsproxy;
/* signal handlers */
	struct signal_struct *signal;
	struct sighand_struct *sighand;
 
	sigset_t blocked, real_blocked;
	sigset_t saved_sigmask;		/* To be restored with TIF_RESTORE_SIGMASK */
	struct sigpending pending;
 
	unsigned long sas_ss_sp;
	size_t sas_ss_size;
	int (*notifier)(void *priv);
	void *notifier_data;
	sigset_t *notifier_mask;
	
	void *security;
	struct audit_context *audit_context;
	seccomp_t seccomp;
 
/* Thread group tracking */
   	u32 parent_exec_id;
   	u32 self_exec_id;
/* Protection of (de-)allocation: mm, files, fs, tty, keyrings */
	spinlock_t alloc_lock;
 
	/* Protection of the PI data structures: */
	spinlock_t pi_lock;
 
#ifdef CONFIG_RT_MUTEXES
	/* PI waiters blocked on a rt_mutex held by this task */
	struct plist_head pi_waiters;
	/* Deadlock detection and priority inheritance handling */
	struct rt_mutex_waiter *pi_blocked_on;
#endif
 
#ifdef CONFIG_DEBUG_MUTEXES
	/* mutex deadlock detection */
	struct mutex_waiter *blocked_on;
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	unsigned int irq_events;
	int hardirqs_enabled;
	unsigned long hardirq_enable_ip;
	unsigned int hardirq_enable_event;
	unsigned long hardirq_disable_ip;
	unsigned int hardirq_disable_event;
	int softirqs_enabled;
	unsigned long softirq_disable_ip;
	unsigned int softirq_disable_event;
	unsigned long softirq_enable_ip;
	unsigned int softirq_enable_event;
	int hardirq_context;
	int softirq_context;
#endif
#ifdef CONFIG_LOCKDEP
# define MAX_LOCK_DEPTH 30UL
	u64 curr_chain_key;
	int lockdep_depth;
	struct held_lock held_locks[MAX_LOCK_DEPTH];
	unsigned int lockdep_recursion;
#endif
 
/* journalling filesystem info */
	void *journal_info;
 
/* stacked block device info */
	struct bio *bio_list, **bio_tail;
 
/* VM state */
	struct reclaim_state *reclaim_state;
 
	struct backing_dev_info *backing_dev_info;
 
	struct io_context *io_context;
 
	unsigned long ptrace_message;
	siginfo_t *last_siginfo; /* For ptrace use.  */
/*
 * current io wait handle: wait queue entry to use for io waits
 * If this thread is processing aio, this points at the waitqueue
 * inside the currently handled kiocb. It may be NULL (i.e. default
 * to a stack based synchronous wait) if its doing sync IO.
 */
	wait_queue_t *io_wait;
#ifdef CONFIG_TASK_XACCT
/* i/o counters(bytes read/written, #syscalls */
	u64 rchar, wchar, syscr, syscw;
#endif
	struct task_io_accounting ioac;
#if defined(CONFIG_TASK_XACCT)
	u64 acct_rss_mem1;	/* accumulated rss usage */
	u64 acct_vm_mem1;	/* accumulated virtual memory usage */
	cputime_t acct_stimexpd;/* stime since last update */
#endif
#ifdef CONFIG_NUMA
  	struct mempolicy *mempolicy;
	short il_next;
#endif
#ifdef CONFIG_CPUSETS
	struct cpuset *cpuset;
	nodemask_t mems_allowed;
	int cpuset_mems_generation;
	int cpuset_mem_spread_rotor;
#endif
	struct robust_list_head __user *robust_list;
#ifdef CONFIG_COMPAT
	struct compat_robust_list_head __user *compat_robust_list;
#endif
	struct list_head pi_state_list;
	struct futex_pi_state *pi_state_cache;
 
	atomic_t fs_excl;	/* holding fs exclusive resources */
	struct rcu_head rcu;
 
	/*
	 * cache last used pipe for splice
	 */
	struct pipe_inode_info *splice_pipe;
#ifdef	CONFIG_TASK_DELAY_ACCT
	struct task_delay_info *delays;
#endif
#ifdef CONFIG_FAULT_INJECTION
	int make_it_fail;
#endif
};
```
### 进程状态
> 在linux中进程有6种状态：
#### R状态
> 可执行状态(running)，此状态是指进程可以运行，表明进程正在运行或在运行队列中等待运行，只有该状态的进程才可能在CPU上运行。
#### S状态
> 睡眠状态(sleeping)，可中断的睡眠状态，处于此状态的进程因等待某事件的发生而被挂起，这些进程的PCB被放入等待队列，当所等待的事件发生时，对应等待队列中的进程将被唤醒。
#### D状态
> 磁盘休眠状态(disk sleep)，不可中断的睡眠状态，进程此时处于睡眠，但不可以被系统杀死，通常此时进程在等待I/O结束。
#### T状态
> 停止状态(stopped)，暂停状态或者跟踪状态，当进程收到SIGSTOP信号后进入此状态，发送SIGCONT信号可以使进程继续运行。
#### Z状态
> 僵尸状态(zombies)， 表示一个进程即将死亡的状态，等待其父进程回收其资源。当子进程退出并且父进程没有读取到子进程退出的返回代码时，就会产生僵尸进程。
```
僵尸状态是一个比较特殊的状态
僵死进程会以终止状态保持在进程表中，并且会一直在等待父进程读取退出状态代码。所以，只要子进程退出，父进程还在运行，但父进程没有读取子进程状态，子进程进入Z状态。
当一个子进程运行结束后，其父进程或者操作系统必须得知子进程的任务完成情况，所以当子进程退出时其信息不会立即释放，存放在PCB中，而父进程不会来读取，此时子进程已经结束，就会进入僵尸状态。
```
```
如果父进程一直不读取子进程的退出返回信息，子进程就会一直维持此状态，PCB就要一直维护此信息，不会被释放，这样就会造成内存空间资源的浪费，容易造成内存泄漏。
```
#### X状态
> 死亡状态(dead)，这个状态是一个返回状态，当父进程读取子进程的返回结果后，子进程会立即释放资源，彻底终止，从系统中消失，所以此状态几乎不可能被捕捉到。
#### 孤儿进程
> 当一个子进程仍在运行，但是它的父进程已经结束退出，这个进程就会成为孤儿进程，孤儿进程会被`init`进程(PID=1)收养，此后孤儿进程的状态和最后的PCB空间释放都是由`init`进程负责了。





### 进程的创建与终止
- **创建**：Linux采用独特的fork()系统调用来创建进程，这是一种“复制”父进程的方式来创建新进程。此外，还有exec()系列函数用于执行新的程序，替换当前进程的内存映像。
- **终止**：进程可以通过调用exit()或_exit()系统调用来终止自己。父进程可以使用wait()或waitpid()系统调用来等待子进程的结束并回收其资源。
### 进程的调度
- **定义**：进程的调度是指内核决定哪个进程可以获得CPU的使用权。
- **策略**：Linux采用了多种调度策略，如CFS（Completely Fair Scheduler）等，以实现公平和高效的进程调度。
### 进程间通信
- **定义**：IPC是指不同进程之间传递信息或数据的过程。
- **方式**：Linux支持多种IPC方式，如管道、消息队列、信号量、共享内存等。
### 进程管理命令
- **ps**：用于查看当前系统中的进程状态。
- **top**：实时显示系统中各个进程的资源占用状况。
- **kill**：用于向进程发送信号，以终止进程或改变进程的状态。
