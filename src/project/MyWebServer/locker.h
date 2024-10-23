//线程同步包装类
#ifndef LOCKER_H
#define LOCKER_H

#include<exception>
#include<pthread.h>
#include<semaphore.h>
class locker
{
    public:
        //创建并初始化互斥锁
        locker()
        {
            if(pthread_mutex_init(&m_mutex, NULL)!=0)
            {
                throw std::exception();
            }
        }
        //销毁互斥锁
        ~locker()
        {
            pthread_mutex_destroy(&m_mutex);
        }
        //获取互斥锁
        bool lock()
        {
            return pthread_mutex_lock(&m_mutex)==0;
        }
        //释放互斥锁
        bool unlock()
        {
            return pthread_mutex_unlock(&m_mutex)==0;
        }
        //获取互斥锁成员变量
        pthread_mutex_t* get()
        {
            return &m_mutex;
        }
    
    private:
        pthread_mutex_t m_mutex;    //互斥锁变量
};
class sem
{
    public:
        //初始值为0的信号量
        sem()
        {
            if(sem_init(&m_sem, 0, 0)!=0)
                throw std::exception();
        }
        //指定初始值的信号量
        sem(int num)
        {
            if(sem_init(&m_sem, 0, num)!=0)
                throw std::exception();
        }
        ~sem()
        {
            sem_destroy(&m_sem);
        }
        bool wait()
        {
            return sem_wait(&m_sem)==0;
        }
        bool post()
        {
            return sem_post(&m_sem)==0;
        }
    private:
        sem_t m_sem;
};
#endif