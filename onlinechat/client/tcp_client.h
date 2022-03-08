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
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "msg.h"
#include "RC4.h"

#define FileUnExit -1 //文件不存在
#define FileExit 0    //文件存在

#define UNLOGIN -1 //未登录
#define UNFORBID 0 //未被禁言
#define FORBID 1   //被禁言
#define UNADMIN 0  //普通用户
#define ADMIN 1    //管理员

char server_ip[16];
int port; //全局变量

// //用于本地判断
char Client_Name[20]; //用户名
int Client_Id;        //登录id (1起始)
int connfd;           //被禁言标志0/1
int admin;            //普通用户0 管理员1
int file_to_cfd;      //传文件目标cfd
int actionFlag;

char Client_FileName[30];

char *mbegin; // map起始地址
struct stat filestat;
struct file_info finfo;
int fd;
int last_bs;
char id_buf[4];
int freeid;

int c2_recv;

pthread_t id;
pthread_t id2;
sem_t sems[2];

pthread_mutex_t mutex; //锁


//初始化客户端,返回sockfd
int Client_init(char *ip);

void *thread_read(void *arg); //线程函数

void menu();                              //界面
void regist(Msg *pmsg, int sockfd);       //注册
void login(Msg *pmsg, int sockfd);        //登录
void pricate_chat(Msg *pmsg, int sockfd); //私聊
void group_chat(Msg *pmsg, int sockfd);   //群聊
void display(Msg *pmsg, int sockfd);      //显示所有在线用户
void forbid(Msg *pmsg, int sockfd);       //禁言
void relieve(Msg *pmsg, int sockfd);      //解除禁言
void user_exit(Msg *pmsg, int sockfd);    //用户下线
void kick_out(Msg *pmsg, int sockfd);     //踢出用户
int file_exist(const char *file_path);    //判断文件是否存在
void sfile_check(Msg *pmsg, int sockfd);  //发文件,需要接收人确认
int send_file(Msg *pmsg, int sockfd);     //发文件
int get_file_size(const char *file_path); //获取文件信息
void send_fileinfo(int sockfd, Msg *pmsg, const char *file_path, struct stat *p_fstat, struct file_info *p_finfo);
void passwd_recovery(Msg *pmsg, int sockfd); //找回密码
void cheak_history(Msg *pmsg, int sockfd);   //查看历史记录
void my_name();                              //显示当前用户
void SendBigFile(Msg *pmsg, int sockfd);     //传大文件

//接收服务器分配ID,文件切块,创建多线程发送文件块
void FileIntend(Msg *pmsg, int sockfd);

//发送文件块
void *send_filedata(void *args);

//生成文件块头部
struct file_head *new_fb_head(char *filename, int freeid, int *offset);

//客户端2接收到收文件的请求 和 文件信息
void client_2_recv_check(Msg *pmsg, int cfd);

//创建大小为size的文件
int createfile(char *filename, int size);

//客户端2多线程读取文件
void *client_2_recv_file(void *args);