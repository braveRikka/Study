#include "../include/thread_pool.h"

void *threadpool_function(void *arg)
{
    struct threadpool *pool = (struct threadpool *)arg;
    struct job *pjob = NULL;
    while (1)
    {
        pthread_mutex_lock(&(pool->mutex));

        while (pool->queue_cur_num == 0)
        {
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->mutex));
        }
        pjob = pool->head;
        pool->queue_cur_num--;
        if (pool->queue_cur_num != pool->queue_max_num)
        {
            pthread_cond_broadcast(&(pool->queue_not_full));
        }

        if (pool->queue_cur_num == 0)
        {
            pool->head = pool->tail = NULL;
            pthread_cond_broadcast(&(pool->queue_empty));
        }
        else
        {
            pool->head = pjob->next;
        }
        pthread_mutex_unlock(&(pool->mutex));

        (*(pjob->func))(pjob->arg);
        free(pjob);
        pjob = NULL;
    }
}

struct threadpool *threadpool_init(int thread_num, int queue_max_num)
{

    struct threadpool *pool = (struct threadpool *)malloc(sizeof(struct threadpool));

    pool->queue_max_num = queue_max_num;
    pool->queue_cur_num = 0;
    pool->head = NULL;
    pool->tail = NULL;

    pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->queue_empty), NULL);
    pthread_cond_init(&(pool->queue_not_empty), NULL);
    pthread_cond_init(&(pool->queue_not_full), NULL);

    pool->thread_num = thread_num;
    pool->pthread_ids = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);

    for (int i = 0; i < pool->thread_num; i++)
    {
        pthread_create(&(pool->pthread_ids[i]), NULL, threadpool_function, (void *)pool);
    }

    return pool;
}

void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg)
{
    pthread_mutex_lock(&(pool->mutex));
    while (pool->queue_cur_num == pool->queue_max_num)
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex));
    }

    struct job *pjob = (struct job *)malloc(sizeof(struct job));

    pjob->func = func;
    pjob->arg = arg;
    pjob->next = NULL;

    if (pool->head == NULL)
    {
        //任务队列为空
        pool->head = pool->tail = pjob;
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    else
    {
        pool->tail->next = pjob;
        pool->tail = pjob;
    }

    pool->queue_cur_num++;
    pthread_mutex_unlock(&(pool->mutex));
}

void thread_destory(struct threadpool *pool)
{
    pthread_mutex_lock(&(pool->mutex));
    while (pool->queue_cur_num != 0)
    {
        pthread_cond_wait(&(pool->queue_empty), &(pool->mutex));
    }

    pthread_mutex_unlock(&(pool->mutex));
    pthread_cond_broadcast(&(pool->queue_not_empty));
    pthread_cond_broadcast(&(pool->queue_not_full));

    for (int i = 0; i < pool->thread_num; i++)
    {
        //释放资源
        pthread_join(pool->pthread_ids[i], NULL);
    }
    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
    free(pool->pthread_ids);

    struct job *temp;
    while (pool->head != NULL)
    {
        temp = pool->head;
        pool->head = temp->next;
        free(temp);
    }
    free(pool);
}

// void *work(void *arg)
// {
//     printf("bbbbbbb\n");
//     // char *p = (char *)arg;
//     // printf("hello world %s\n", p);
//     printf("ccccccc\n");
//     sleep(1);
// }

// int main(int argc, char const *argv[])
// {
//     struct threadpool *pool = threadpool_init(20, 100);

//     threadpool_add_job(pool, work, "1");
//     threadpool_add_job(pool, work, "2");
//     threadpool_add_job(pool, work, "3");
//     threadpool_add_job(pool, work, "4");
//     threadpool_add_job(pool, work, "5");
//     sleep(5);

//     thread_destory(pool);

//     return 0;
// }
