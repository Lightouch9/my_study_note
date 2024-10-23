//头文件包含重复性检查
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<list>
#include<cstdio>
#include<exception>
#include<pthread.h>

#include"locker.h"
template<typename T>
class threadpool
{
    public:
        //构造与析构
        threadpool(int actor_model, int thread_num, int max_requests=10000);
        ~threadpool();
        bool append(T* request, int state);  //向任务队列中添加任务
        bool append_p(T* request);
    private:
        //成员变量
        std::list<T*> m_work_queue; //任务队列，存储待处理的任务
        locker m_queuelocker;   //保护任务队列的互斥锁
        sem m_queuesem; //指示是否有任务需要处理的信号量
        int m_max_request;  //任务队列中允许的最大请求数

        pthread_t* m_threads;   //描述线程池的数组，大小为m_thread_num
        int m_thread_num;   //线程池中的线程数

        int m_actor_model;    //是否启用actor工作模式
    private:
        //成员函数
        static void* worker(void* arg); //工作线程运行的函数，会不断从任务队列中取出任务执行
        void run();


};
//线程池构造函数
template<typename T>
threadpool<T>::threadpool(int actor_model, int thread_num, int max_requests=10000) : m_thread_num(thread_num), m_max_request(max_requests), m_actor_model(actor_model)
{
    //线程数量与最大请求任务数量合法性检查
    if(thread_num<=0||max_requests<=0)
        throw std::exception();
    m_threads=new pthread_t[m_thread_num];
    //错误检查
    if(!m_threads)
        throw std::exception();
    //创建thread_num个子线程，并设置为脱离线程
    for(int i=0;i<thread_num;i++)
    {
        if(pthread_create(m_threads+i, NULL, worker, this)!=0)
        {
            //创建失败
            delete[] m_threads;
            throw std::exception();
        }
        //分离子线程
        if(pthread_detach(m_threads[i]))
        {
            //错误处理
            delete[] m_threads;
            throw std::exception();
        }
    }
}
//析构函数
template<typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}
//向任务队列中添加任务
template<typename T>
bool threadpool<T>::append(T* request, int state)
{
    //任务队列互斥锁获取
    m_queuelocker.lock();
    //如果任务队列中的任务数量已经达到限制
    if(m_work_queue.size()>=m_max_request)
    {
        //释放锁
        m_queuelocker.unlock();
        return false;
    }
    //设置任务的读写类型
    reqest->m_state=state;
    //加入任务队列
    m_work_queue.push_back(request);
    //临界区结束释放锁
    m_queuelocker.unlock();
    //添加任务后，信号量加1，指示任务队列中已有任务待处理
    m_queuesem.post();
    return true;
}
//向任务队列中添加任务
template<typename T>
bool threadpool<T>::append_p(T* request)
{
    //互斥地访问任务队列，因为任务队列被多个线程共享
    //获取锁
    m_queuelocker.lock();
    //如果任务队列中的任务数量已经达到限制
    if(m_work_queue.size()>=m_max_request)
    {
        //释放锁
        m_queuelocker.unlock();
        return false;
    }
    //加入任务队列
    m_work_queue.push_back(request);
    //临界区结束释放锁
    m_queuelocker.unlock();
    //添加任务后，信号量加1，指示任务队列中已有任务待处理
    m_queuesem.post();
    return true;
}
//工作线程的运行函数，从任务队列中获取任务并处理
template<typename T>
void* threadpool<T>::worker(void* arg)
{
    //获取传入的线程池对象
    threadpool* pool=(threadpool*)arg;
    pool->run();
    return pool;
}
template<typename T>
void threadpool<T>::run()
{
    //循环获取任务队列中的任务
    while(true)
    {
        //获取指示任务队列中是否有任务的信号量
        m_queuesem.wait();
        //获取任务队列的互斥锁
        m_queuelocker.lock();
        //任务队列非空检查
        if(m_work_queue.empty())
        {
            //队列空则释放锁，重新循环
            m_queuelocker.unlock();
            continue;
        }
        //获取队列首部的任务
        T* request = m_work_queue.front();
        //弹出任务
        m_work_queue.pop_front();
        //临界区结束，释放锁
        m_queuelocker.unlock();
        //任务非空检查
        if(!request)
            continue;
        //工作模式判断
        //启用reactor
        if(m_actor_model==1)
        {
            //判断任务类型
            //读
            if(request->m_state==0)
            {
                if(request->read_once())
                {
                    request->improv=1;
                    request->process();
                }
                //读失败
                else
                {
                    request->improv=1;
                    request->timer_flag=1;  //标识要关闭连接
                }
            }
            //写
            else
            {
                if(request->write())
                {
                    request->improv=1;
                }
                else
                {
                    request->improv=1;
                    request->timer_flag=1;
                }
            }
        }
        //proactor
        else
        {
            request->process();
        }
    }
}

#endif