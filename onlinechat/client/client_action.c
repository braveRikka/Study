#include "tcp_client.h"
#include "msg.h"
#include "RC4.h"

#define CONN_MAX 10
struct conn gconn[CONN_MAX];
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER; //初始化锁

void menu() //主菜单函数//
{
    printf("%s", "\033[1H\033[2J"); //清屏

    printf("\n\n\n\n\n"); //控制主功能菜单显示位置//
    printf("\t\t|--------------------------------聊-天-室--------------------|\t\n");
    printf("\t\t|exit       退出                                             |\t\n");
    printf("\t\t|reg        注册                                             |\t\n");
    printf("\t\t|log        登录                                             |\t\n");
    printf("\t\t|name       显示当前用户名                                    |\t\n");
    printf("\t\t|forget     找回密码                                         |\t\n");
    printf("\t\t|user       显示在线用户                                      |\t\n");
    printf("\t\t|chat       私聊                                             |\t\n");
    printf("\t\t|group      群聊                                             |\t\n");
    printf("\t\t|history    查看历史记录                                      |\t\n");
    printf("\t\t|forbid     禁言                                             |\t\n");
    printf("\t\t|relieve    解禁                                             |\t\n");
    printf("\t\t|kick       踢出                                             |\t\n");
    printf("\t\t|send       发送文件                                         |\t\n");
    printf("\t\t|help/menu  菜单                                             |\t\n");
    printf("\t\t|------------------------------------------------------------|\t\n");
    printf("\t\t\t输入操作:");
}

void regist(Msg *pmsg, int sockfd)
{
    if (Client_Id != UNLOGIN)
    {
        printf("您已登录,请勿重复注册\n");
        return;
    }

    memset(pmsg, 0, sizeof(Msg));
    pmsg->action = 1;

    printf("请输入用户名:\n");
    scanf("%s", pmsg->name);
    getchar(); //吞掉输入错误的操作

    printf("请输入密码:\n");
    scanf("%s", pmsg->password);
    getchar(); //吞掉输入错误的操作

    printf("请输入密保问题\n");
    scanf("%s", pmsg->question);
    getchar(); //吞掉输入错误的操作

    printf("请输入密保答案\n");
    scanf("%s", pmsg->answer);
    getchar(); //吞掉输入错误的操作

    printf("请选择是否注册管理员(y/n)\n");
    char ch;
    scanf(" %c", &ch);
    getchar(); //吞掉输入错误的操作

    if ((ch == 'y') || (ch == 'Y'))
    {
        pmsg->admin = 1;
    }
    else
    {
        pmsg->admin = 0;
    }
    pmsg->connfd = 0;
    if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }

    // printf("client send pmsg->action = %d\n", pmsg->action);
}

void login(Msg *pmsg, int sockfd)
{
    if (Client_Id != UNLOGIN)
    {
        printf("您已登录,请勿重复登录\n");
        return;
    }

    memset(pmsg, 0, sizeof(Msg));
    pmsg->action = 2;

    // printf("请输入用户名:\n");
    // scanf("%s", pmsg->name);

    printf("请输入用户id:\n");
    scanf("%d", &pmsg->id);
    getchar(); //吞掉输入错误的操作

    printf("请输入密码:\n");
    scanf("%s", pmsg->password);
    getchar(); //吞掉输入错误的操作

    // printf("pmsg->action = %d\n", pmsg->action);
    if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }

    // printf("client send pmsg->action = %d\n", pmsg->action);
}

void pricate_chat(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (connfd == FORBID)
        {
            printf("您已被禁言\n");
            return;
        }
        else if (connfd == UNFORBID)
        {
            memset(pmsg, 0, sizeof(Msg));
            pmsg->action = 3;
            printf("请输入私聊对象:\n");
            scanf("%s", pmsg->toname);
            getchar(); //吞掉输入错误的操作

            // if (strcmp(pmsg->toname,Client_Name) == 0)
            // {
            //     //私聊对象为自己,服务端会做判断
            //     //本地看后序要不要额外操作
            // }

            printf("请输入聊天内容:\n");
            char source[1024] = {0};
            scanf("%s", source);
            RC4Encrypt(Client_Name, source, pmsg->message);
            getchar(); //吞掉输入错误的操作

            if (write(sockfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
            // printf("client send pmsg->action = %d\n", pmsg->action);
        }
    }
}

void group_chat(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (connfd == FORBID)
        {
            printf("您已被禁言\n");
            return;
        }
        else if (connfd == UNFORBID)
        {
            memset(pmsg, 0, sizeof(Msg));
            pmsg->action = 4;
            printf("请输入聊天内容:\n");
            char source[1024] = {0};
            scanf("%s", source);
            RC4Encrypt(Client_Name, source, pmsg->message);

            getchar(); //吞掉输入错误的操作

            if (write(sockfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }

            // printf("client send pmsg->action = %d\n", pmsg->action);
        }
    }
}

void display(Msg *pmsg, int sockfd)
{
    //显示所有在线用户
    memset(pmsg, 0, sizeof(Msg));
    pmsg->action = 5;
    if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

void forbid(Msg *pmsg, int sockfd) //禁言
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (admin == UNADMIN)
        {
            printf("您不是管理员,无法禁言用户\n");
            return;
        }
        else if (admin == ADMIN) //是管理员
        {
            memset(pmsg, 0, sizeof(Msg));
            pmsg->action = 7;
            printf("请输入禁言对象:\n");
            scanf("%s", pmsg->toname);
            getchar(); //吞掉输入错误的操作

            // if (strcmp(pmsg->toname, Client_Name) == 0)
            // {
            //     //禁言对象为自己,服务端会做判断
            //     //本地看后序要不要额外操作
            // }

            if (write(sockfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }

            // printf("client send pmsg->action = %d\n", pmsg->action);
        }
    }
}

void relieve(Msg *pmsg, int sockfd) //解除禁言
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (admin == UNADMIN)
        {
            printf("您不是管理员,无法解禁用户\n");
            return;
        }
        else if (admin == ADMIN) //是管理员
        {
            memset(pmsg, 0, sizeof(Msg));
            pmsg->action = 8;
            printf("请输入解禁对象:\n");
            scanf("%s", pmsg->toname);
            getchar(); //吞掉输入错误的操作

            if (write(sockfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }

            // printf("client send pmsg->action = %d\n", pmsg->action);
        }
    }
}

//用户下线
void user_exit(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录,可以退出
    {
        memset(pmsg, 0, sizeof(Msg));
        pmsg->action = 11;
        memset(Client_Name, 0, 20); //用户名清空
        Client_Id = -1;             //登录id (1起始)
        connfd = 0;                 //被禁言标志0/1
        admin = 0;                  //普通用户0 管理员1
        if (write(sockfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }

        // printf("client send pmsg->action = %d\n", pmsg->action);
    }
}

//踢出用户
void kick_out(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (admin == UNADMIN)
        {
            printf("您不是管理员,无法踢出用户\n");
            return;
        }
        else if (admin == ADMIN) //是管理员
        {
            memset(pmsg, 0, sizeof(Msg));
            pmsg->action = 9;
            printf("请输入踢出对象:\n");
            scanf("%s", pmsg->toname);
            getchar(); //吞掉输入错误的操作

            if (write(sockfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }

            // printf("client send pmsg->action = %d\n", pmsg->action);
        }
    }
}

//发文件,需要接收人确认
void sfile_check(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (connfd == FORBID)
        {
            printf("您已被禁言\n");
            return;
        }
        else if (connfd == UNFORBID) //未被禁言
        {
            //输入发送文件的对象
            memset(pmsg, 0, sizeof(Msg));
            pmsg->action = 12;
            printf("请输入发送文件对象:\n");
            scanf("%s", pmsg->toname);
            getchar(); //吞掉输入错误的操作

            if (write(sockfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
            // printf("client send pmsg->action = %d\n", pmsg->action);
        }
    }
}

//判断文件是否存在
int file_exist(const char *file_path)
{
    if (file_path == NULL) //文件不存在
    {
        return FileUnExit;
    }
    if (access(file_path, F_OK) == 0) //文件存在
    {
        return FileExit;
    }
    return FileUnExit;
}

//发文件
int send_file(Msg *pmsg, int sockfd)
{
    printf("请输入要发送的文件名:\n");
    char file_path[1024] = {0};

    scanf("%s", file_path);
    getchar(); //吞掉输入错误的操作

    if (file_exist(file_path) == FileUnExit) //文件不存在
    {
        printf("文件:%s 不存在\n", file_path);
        return -1;
    }

    //发送文件所用时间
    //获取文件信息(大小)
    int file_size = -1;
    file_size = get_file_size(file_path);

    int from_fd;

    // char buf[2048] = {0};
    from_fd = open(file_path, O_RDONLY);

    lseek(from_fd, 0, SEEK_SET);

    pmsg->action = 14;
    strcpy(pmsg->file_path, file_path);
    int ret = 0;
    while ((ret = read(from_fd, pmsg->file, 1024)) > 0)
    {
        if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
        memset(pmsg->file, 0, 1024);
    }

    // pmsg->file_size = file_size;

    close(from_fd);

    return 0;
}

//获取文件信息
int get_file_size(const char *file_path)
{
    int file_size = -1;
    struct stat file;
    if (stat(file_path, &file) < 0)
    {
        return file_size;
    }
    return file.st_size;
}

void send_fileinfo(int sockfd, Msg *pmsg, const char *file_path, struct stat *p_fstat, struct file_info *p_finfo)
{
    //初始化file_info
    int info_len = sizeof(struct file_info);
    memset(p_finfo, 0, info_len);
    strcpy(p_finfo->file_name, file_path);
    p_finfo->file_size = p_fstat->st_size;
    // printf("文件大小:%d\n\n", p_finfo->file_size);

    //最后一个分块可能不足一个标准分块
    int count = p_fstat->st_size / BIGBLOCKSIZE;
    if (p_fstat->st_size % BIGBLOCKSIZE == 0)
    {
        p_finfo->count = count; //正好是标准块的整数倍
    }
    else
    {
        p_finfo->count = count + 1;
        last_bs = p_fstat->st_size - BIGBLOCKSIZE * count;
    }
    p_finfo->bs = BIGBLOCKSIZE;

    //发送type和文件信息
    char send_buf[100] = {0};
    memcpy(send_buf, p_finfo, info_len);
    // send(sockfd,pmsg,sizeof(Msg),0);//action = 16
    // write(sockfd, pmsg, sizeof(Msg)); //告诉服务器要传大文件了
    if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }

    // sleep(1);
    send(sockfd, send_buf, info_len, 0); //再发文件信息

    // printf("-------- 文件信息 --------\n");
    // printf("文件名= %s\n文件大小= %d\n分块数= %d\n块大小= %d\n", p_finfo->file_name, p_finfo->file_size, p_finfo->count, p_finfo->bs);
    // printf("-------------------------\n");

    printf("-------- 文件信息 --------\n");
    printf("文件名= %s\n文件大小= %d\n", p_finfo->file_name, p_finfo->file_size);
    printf("-------------------------\n");
}

//找回密码
void passwd_recovery(Msg *pmsg, int sockfd)
{
    memset(pmsg, 0, sizeof(Msg));
    pmsg->action = 6;
    printf("请输入账号:\n");
    scanf("%s", pmsg->name);
    getchar(); //吞掉输入错误的操作

    printf("请输入密保问题:\n");
    scanf("%s", pmsg->question);
    getchar(); //吞掉输入错误的操作

    printf("请输入密保答案:\n");
    scanf("%s", pmsg->answer);
    getchar(); //吞掉输入错误的操作

    printf("请输入新密码:\n");
    scanf("%s", pmsg->password);
    getchar(); //吞掉输入错误的操作

    // write(sockfd, pmsg, sizeof(Msg));
    if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//显示当前用户
void my_name()
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录,可以查看当前用户名
    {
        printf("当前用户名:%s\n", Client_Name);
    }
}

//查看历史记录
void cheak_history(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录,可以查看历史记录
    {
        memset(pmsg, 0, sizeof(Msg));
        pmsg->action = 15;
        strcmp(pmsg->name, Client_Name);
        // write(sockfd, pmsg, sizeof(Msg));
        if (write(sockfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
    }
}

//传大文件
void SendBigFile(Msg *pmsg, int sockfd)
{
    if (Client_Id == UNLOGIN)
    {
        printf("您未登录,请先登录\n");
        return;
    }
    else if (Client_Id > 0) //已登录
    {
        if (connfd == FORBID)
        {
            printf("您已被禁言\n");
            return;
        }
        else if (connfd == UNFORBID)
        {
            //待会写完了丢这里
            //懒得丢
        }
    }
    memset(pmsg, 0, sizeof(Msg));
    strcpy(pmsg->name, Client_Name); //发送方姓名
    pmsg->action = 16;               //发给服务器的动作判断 action = 16
    printf("请输入发送文件对象:\n");
    scanf("%s", pmsg->toname); //接收方姓名
    getchar();                 //吞掉输入错误的操作

    // printf("BIGBLOCKSIZE=  %d\n", BIGBLOCKSIZE); //大文件标准块大小
    char filename[30] = {0}; //文件名
    printf("请输入文件名:");
    scanf("%s", filename);
    //判断文件是否存在
    if (file_exist(filename) == FileUnExit)
    {
        printf("文件不存在\n");
        return;
    }
    // else if (file_exist(filename) == FileExit)
    // {
    //     ;
    // }
    strcpy(Client_FileName, filename);
    // printf("client_filename = %s\n", Client_FileName);
    fd = open(filename, O_RDWR);
    //发送 请求、文件信息 给服务器
    fstat(fd, &filestat);
    send_fileinfo(sockfd, pmsg, filename, &filestat, &finfo);
}

//文件切块,创建多线程发送文件块
void FileIntend(Msg *pmsg, int sockfd)
{
    // map，关闭fd
    mbegin = (char *)mmap(NULL, filestat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    struct fun_arg arg;
    Msg tp;
    memcpy(&tp, pmsg, sizeof(Msg));
    arg.pmsg = &tp;

    printf("正在发送文件...\n");
    //向任务队列添加任务
    int j = 0;
    int num = finfo.count, offset = 0;
    pthread_t pid[num];
    memset(pid, 0, num * sizeof(pthread_t));
    int head_len = sizeof(struct file_head);
    //文件可以被标准分块
    if (last_bs == 0)
    {
        for (j = 0; j < num; j++)
        {
            sleep(1);
            struct file_head *p_fhead = new_fb_head(Client_FileName, freeid, &offset);
            arg.p_fhead = p_fhead;
            if (pthread_create(&pid[j], NULL, send_filedata, &arg) != 0)
            {
                printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
                // printf("616\n");
                free(p_fhead);
                exit(-1);
            }
            // free(p_fhead);
        }
    }
    //文件不能被标准分块
    else
    {
        for (j = 0; j < num - 1; j++)
        {
            sleep(1);
            // printf("j = %d\n", j);
            struct file_head *p_fhead = new_fb_head(Client_FileName, freeid, &offset);
            arg.p_fhead = p_fhead;
            if (pthread_create(&pid[j], NULL, send_filedata, &arg) != 0)
            {
                printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
                // printf("635\n");
                free(p_fhead);
                exit(-1);
            }
            // printf("p_fhead->name = %s\n", p_fhead->file_name);
            // printf("arg.p_fhead->file_name = %s\n", arg.p_fhead->file_name);
            // sleep(1);
            // free(p_fhead);
        }
        //最后一个分块
        struct file_head *p_fhead = (struct file_head *)malloc(head_len);
        bzero(p_fhead, head_len);
        strcpy(p_fhead->file_name, Client_FileName);
        p_fhead->id = freeid;
        p_fhead->offset = offset;
        p_fhead->bs = last_bs;
        arg.p_fhead = p_fhead;
        // printf("最后一个分块检查多线程前信息\n");
        // printf("文件名= %s 文件块id= %d\n偏移量= %d 实际块大小= %d\n\n", p_fhead->file_name, p_fhead->id, p_fhead->offset, p_fhead->bs);

        sleep(1);
        if (pthread_create(&pid[j], NULL, send_filedata, &arg) != 0)
        {
            printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
            // free(p_fhead);
            exit(-1);
        }
        // free(p_fhead);
    }

    //回收线程
    for (j = 0; j < num; ++j)
    {
        pthread_join(pid[j], NULL);
    }

    printf("-----------------  发送完毕 ----------------- \n\n请继续输入cmd:\n");
}

//生成文件块头部
struct file_head *new_fb_head(char *filename, int freeid, int *offset)
{
    int head_len = sizeof(struct file_head);
    struct file_head *head_info = (struct file_head *)malloc(head_len);
    memset(head_info, 0, head_len);
    strcpy(head_info->file_name, filename);
    head_info->id = freeid;
    head_info->offset = *offset;
    head_info->bs = BIGBLOCKSIZE;
    *offset += BIGBLOCKSIZE;
    return head_info;
}

//发送文件块
void *send_filedata(void *args)
{
    struct fun_arg arg;
    arg = *(struct fun_arg *)args;

    int head_len = sizeof(struct file_head);

    Msg pmsg = *(arg.pmsg);

    struct file_head fl_head;
    memcpy(&fl_head, arg.p_fhead, head_len);
    struct file_head *s_fhead = &fl_head;

    //这玩意为啥会free错误啊
    // free(arg.p_fhead);
    // arg.p_fhead = NULL;
    int sock_fd = Client_init(server_ip);
    // set_fd_noblock(sock_fd);

    //发给服务器action = 19 让服务器接收文件块
    pmsg.action = 19;
    sleep(3);
#if 1
    send(sock_fd, &pmsg, sizeof(Msg), 0);
    send(sock_fd, &fl_head, head_len, 0);

    int i = 0, send_size = 0;
    int remain_size = s_fhead->bs;
    char *fp = mbegin + s_fhead->offset;
    int err = s_fhead->bs % SEND_SIZE;
    while (remain_size > 0)
    {
        if ((send_size = send(sock_fd, fp, SEND_SIZE, 0)) > 0)
        {
            fp += send_size;
            remain_size -= send_size;
        }
        if (remain_size <= err)
        {
            send_size = send(sock_fd, fp, remain_size, 0);
            fp += send_size;
            remain_size -= send_size;
        }
    }
    close(sock_fd);
    return NULL;
#endif
}

//客户端2接收到收文件的请求 和 文件信息
void client_2_recv_check(Msg *pmsg, int cfd)
{
    //接收文件信息
    char fileinfo_buf[100] = {0};
    int info_len = sizeof(struct file_info);
    memset(fileinfo_buf, 0, info_len);
    int n = 0;
    for (n = 0; n < info_len; n++)
    {
        recv(cfd, &fileinfo_buf[n], 1, 0);
    }
    struct file_info finfo;
    memcpy(&finfo, fileinfo_buf, info_len);

    //是否接收文件
    printf("-------- 文件信息 --------\n");
    printf("文件名= %s\n文件大小= %d\n", finfo.file_name, finfo.file_size);
    printf("-------------------------\n");
    printf("用户:%s 向您发送文件,是否接收(y/n)\n", pmsg->name);

    sem_wait(&sems[0]);

    if (c2_recv == 1)
    {
        pmsg->file_request = 0; //同意接收
    }
    else if (c2_recv == 2)
    {
        pmsg->file_request = 1; //拒绝接收
    }
    //接收方 确认或者拒绝 返回服务器
    pmsg->action = 18;
    // write(cfd, pmsg, sizeof(Msg));
    if (write(cfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
    send(cfd, &finfo, info_len, 0); //文件信息
    if (pmsg->file_request == 0)
    {
        //创建填充文件，map到虚存
        char filepath[100] = {0};
        strcpy(filepath, finfo.file_name);
        createfile("copy_file", finfo.file_size);
        // createfile(filepath, finfo.file_size);
        int tmpfd = 0;
        if ((tmpfd = open("copy_file", O_RDWR)) == -1)
        {
            printf("open file erro\n");
            exit(-1);
        }
        char *map = (char *)mmap(NULL, finfo.file_size, PROT_WRITE | PROT_READ, MAP_SHARED, tmpfd, 0);
        close(tmpfd);
        pthread_mutex_lock(&conn_lock);

        // printf("接收文件信息,上锁存到数组, enter gconn[]\n");
        while (gconn[freeid].used)
        {
            ++freeid;
            freeid = freeid % 10;
        }
        bzero(&gconn[freeid].filename, 30);
        gconn[freeid].info_fd = cfd;
        strcpy(gconn[freeid].filename, finfo.file_name);
        gconn[freeid].filesize = finfo.file_size;
        gconn[freeid].count = finfo.count;
        gconn[freeid].bs = finfo.bs;
        gconn[freeid].mbegin = map;
        gconn[freeid].recvcount = 0;
        gconn[freeid].used = 1;
        pthread_mutex_unlock(&conn_lock);
        // printf("存完数组,解锁, exit gconn[]\n");
#if 0
        printf("gconn[freeid].info_fd = %d\n", gconn[freeid].info_fd);
        printf("gconn[freeid].filename = %s\n", gconn[freeid].filename);
        printf("gconn[freeid].filesize = %d\n", gconn[freeid].filesize);
        printf("gconn[freeid].count = %d\n", gconn[freeid].count);
        printf("gconn[freeid].bs = %d\n", gconn[freeid].bs);
        printf("gconn[freeid].mbegin = %p\n", gconn[freeid].mbegin);
        printf("gconn[freeid].recvcount = %d\n", gconn[freeid].recvcount);
#endif
        //发送客户端2的分块数组信息     客户端2映射好地址,准备好接收文件块
        char freeid_buf[4] = {0};
        memcpy(freeid_buf, &freeid, 4);
        send(cfd, freeid_buf, 4, 0);

        printf("正在接收文件...\n");
        //客户端2准备接收文件,多线程连接服务器
        int num = finfo.count; //创建进程数
        Msg arg;
        memcpy(&arg, pmsg, sizeof(Msg));
        pthread_t pid[num];
        for (int i = 0; i < num; i++)
        {
            pthread_create(&pid[i], NULL, client_2_recv_file, &arg);
        }
        for (int i = 0; i < num; ++i) //回收线程
        {
            pthread_join(pid[i], NULL);
        }
    }
}

//创建大小为size的文件
int createfile(char *filename, int size)
{
    int tmpfd = open(filename, O_RDWR | O_CREAT, 0655);
    fchmod(tmpfd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    ftruncate(tmpfd, size);
    close(tmpfd);
    return 0;
}

//客户端2多线程读取文件
void *client_2_recv_file(void *args)
{
    //多线程连接服务器
    int sock_fd = Client_init(server_ip);

    Msg pmsg;
    memcpy(&pmsg, (Msg *)args, sizeof(Msg));

    // printf("name = %s\n", pmsg.name);     //客户端1的姓名
    // printf("toname = %s\n", pmsg.toname); //客户端2(本机)的姓名

    pmsg.action = 20;
    send(sock_fd, &pmsg, sizeof(Msg), 0); //发给服务器,填写到sockfd数组

    struct file_head fhead;
    int head_len = sizeof(struct file_head);
    int read_len = recv(sock_fd, &fhead, head_len, 0);
    int recv_id = fhead.id;
    char *fp = gconn[recv_id].mbegin + fhead.offset;

    int remain_size = fhead.bs; //数据块中待接收数据大小
    int err = fhead.bs % RECVBUF_SIZE;
    int size = 0; //一次recv接受数据大小
    // printf("err = %d\n", err);

    while (remain_size > 0)
    {
        if ((size = read(sock_fd, fp, RECVBUF_SIZE)) > 0)
        {
            fp += size;
            remain_size -= size;
        }
        if ((remain_size <= err) && (remain_size > 0))
        {
            // printf("remain_size = %d\n", remain_size);
            // printf("你tmd哪里有毛病\n");
            size = recv(sock_fd, fp, remain_size, 0);
            // printf("size = %d\n", size);
            // printf("这里吗?\n");
            fp += size;
            // printf("还是这里?\n");
            remain_size -= size;
            // printf("给你一拳\n");
            break;
        }
        if (remain_size <= 0)
        {
            // printf("这tmd怎么跳不出来\n");
            break;
        }
    }
    // printf("----------------- 接收到一个文件包 ----------------- \n");

    pthread_mutex_lock(&conn_lock);
    gconn[recv_id].recvcount++;
    if (gconn[recv_id].recvcount == gconn[recv_id].count)
    {
        munmap((void *)gconn[recv_id].mbegin, gconn[recv_id].filesize);

        printf("-----------------  接收到完整文件 ----------------- \n");
        // bzero(&gconn[recv_id], sizeof(struct conn));
        memset(&gconn[recv_id], 0, sizeof(struct conn));
        printf("请继续输入cmd:\n");
    }
    pthread_mutex_unlock(&conn_lock);

    close(sock_fd);
    //接收结束
    return NULL;
}