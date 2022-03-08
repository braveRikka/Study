#pragma once

#define BLOCKSIZE 1024
#define BIGBLOCKSIZE 268435456 //大文件标准块256M
#define RECVBUF_SIZE 65536     // 64K

struct message
{
    int action;
    int id;
    char name[20];
    char password[20];

    char toname[20];
    char message[1024];

    char question[128];
    char answer[128];
    int admin;
    int connfd;

    int file_request;   // 1拒绝接收,默认0
    char file_path[20]; //文件名
    char file[1024];    //文件内容
    int file_size;      //文件大小
};

typedef struct message Msg;

//文件信息
struct file_info
{
    char file_name[30]; //文件名
    int file_size;     //文件大小
    int count;          //分块数量
    int bs;             //标准分块大小
};

//文件分块头部信息
struct file_head
{
    char file_name[30]; //文件名
    int id;             //分块所属文件id
    int offset;         //分块在原文件中的偏移
    int bs;             //本文件分块实际大小
};

//与客户端关联的连接，每次传输建立一个，在多线程之间共享
struct conn
{
    int info_fd;       //信息交换socket：接收文件信息、文件传送通知client
    char filename[30]; //文件名
    int filesize;      //文件大小
    int bs;            //分块大小
    int count;         //分块数量
    int recvcount;     //已接收块数量，recv_count == count表示传输完毕
    char *mbegin;      // mmap起始地址
    int used;          //使用标记，1代表使用，0代表可用
};

//客户端1多线程客户端套接字
struct sockfd_arr
{
    char clientName[20];
    int client_1_sfd;
    int used;
};
struct sockfd_arr CLI_sfd[20];


