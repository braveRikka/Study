#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct job //工作节点
{
    void *arg;
    void *(*func)(void *arg);
    struct job *next;
};

struct threadpool
{
    int thread_num;         //已开启的线程的数量
    pthread_t *pthread_ids; //保存线程池中线程id

    struct job *head;
    struct job *tail;
    int queue_max_num; //任务队列最大数
    int queue_cur_num; //任务队列已有多少个任务

    pthread_mutex_t mutex;
    pthread_cond_t queue_empty;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
};


void *threadpool_function(void *arg);

//初始化线程池
struct threadpool *threadpool_init(int thread_num, int queue_max_num);

//添加任务
void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg);

//销毁线程池
void thread_destory(struct threadpool *pool);

//工作线程
void *work(void *arg);


