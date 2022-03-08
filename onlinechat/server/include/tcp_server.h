#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/mman.h>

#include "thread_pool.h"
#include "msg.h"
#include "database.h"
#include "md5.h"

#define PORT 5555
#define EPOLL_SIZE 50 // epoll监听到的发生变化的文件描述符的最大个数

char *tempbegin;

//在线用户节点
typedef struct onlineuser
{
    int cfd;
    int connfd;    //禁言标志
    int admin;     //管理员标志
    char name[20]; //用户名
    struct onlineuser *front;
    struct onlineuser *next;
} OnlineUser;

OnlineUser *head; //在线用户双向循环链表表头

//创建服务器套接字
int tcp_server_create();
//将文件描述符的属性设置为非阻塞
void set_nonblock(int cfd);

int read_block(int cfd, char *message, size_t size);
int read_nonblcok(int cfd, char *message, size_t size);

//线程函数
void *thread_work(void *arg);

OnlineUser *create_user();                                  //创建一个新节点
void insert_link(OnlineUser *new_user, int cfd, Msg *pmsg); //插入一个节点
void delect_node(OnlineUser *p);                            //双向循环链表删除p节点

//判断是否重复登录,不重复返回0,重复返回1
int ReLogin(Msg *pmsg, int cfd);

