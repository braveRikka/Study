#include "../include/tcp_server.h"

//将文件描述符的属性设置为非阻塞
void set_nonblock(int cfd)
{
    int flags;
    flags = fcntl(cfd, F_GETFL);
    flags = flags | O_NONBLOCK;
    fcntl(cfd, F_SETFL, flags);
}

//创建服务器套接字
int tcp_server_create()
{
    // socket:创建套接字
    //初始化结构体struct sockaddr_in
    // bind:给套接字绑定ip、端口号
    // listen:监听  主动文件描述符变为被动(等待连接)
    // accept:处理连接请求（阻塞）--连接套接字（收发数据）
    // recv\send：接受发送数据(SIGPIPE忽略)
    // shutdown关闭连接

    signal(SIGPIPE, SIG_IGN);

    int sockfd;
    struct sockaddr_in s_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("创建套接字失败!\n");
        exit(1);
    }
    printf("创建套接字成功!\n");

    set_nonblock(sockfd); //设置非阻塞

    int opt = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //设置套接字可以重复使用端口号

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    // s_addr.sin_addr.s_addr = inet_addr(ip);
    s_addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_in)) != 0)
    {
        perror("绑定失败!");
        exit(1);
    }

    printf("绑定成功!\n");

    if (listen(sockfd, 20) != 0)
    {
        perror("监听出错!");
        exit(1);
    }

    head = create_user(); //在线用户双向循环链表表头(全局)
    InitTable();          //初始化数据库和表
    InitHistoryTable();   //初始化历史记录表

    printf("已创建在线用户链表\n");
    printf("监听中...\n");

    return sockfd;
}

int read_nonblcok(int cfd, char *message, size_t size)
{
    int r_n;

    r_n = read(cfd, message, size);

    if (r_n < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        {
            perror("recv message error!");
            exit(1);
        }
    }

    if (r_n == 0)
    {
        printf("client is close!\n");
        return r_n;
    }

    return r_n;
}

int read_block(int cfd, char *message, size_t size)
{
    int r_n;

    r_n = read(cfd, message, size);

    if (r_n < 0)
    {
        perror("recv message error!");
        exit(1);
    }

    if (r_n == 0)
    {
        printf("client is close!\n");
        return r_n;
    }

    return r_n;
}

OnlineUser *create_user()
{
    OnlineUser *user = (OnlineUser *)malloc(sizeof(OnlineUser));
    if (NULL == user)
    {
        perror("malloc error!");
        exit(1);
    }
    user->front = user;
    user->next = user;
    //头尾指向自己
    return user;
}

void insert_link(OnlineUser *new_user, int cfd, Msg *pmsg)
{
    // cfd,用户名,禁言标志,管理员标志
    new_user->cfd = cfd;
    strcpy(new_user->name, pmsg->name);
    new_user->admin = pmsg->admin;
    new_user->connfd = pmsg->connfd;

    new_user->next = head->next;
    new_user->front = head->next->front;
    head->next->front = new_user;
    head->next = new_user;
}

//双向循环链表删除p节点
void delect_node(OnlineUser *p)
{
    p->next->front = p->front;
    p->front->next = p->next;
    free(p);
}

//判断是否重复登录,不重复返回0,重复返回1
int ReLogin(Msg *pmsg, int cfd)
{
    OnlineUser *p = head;
    while (p->next != head)
    {
        p = p->next;
        if (strcmp(p->name, pmsg->name) == 0)
        {
            //在线用户链表中有重复,重复登录，返回1
            return 1;
        }
    }
    return 0;
}
