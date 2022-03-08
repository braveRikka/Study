#include "../include/server_action.h"

//用户注册
void server_regist(Msg *pmsg, int cfd)
{
    printf("用户注册\n");
    printf("用户 %s注册,密码 %s\n", pmsg->name, pmsg->password);

    //注册判断
    int regflag = RegistSelectDatabase(pmsg, cfd);
    if (regflag == 1)
    {
        //该用户已注册,返回登录
        printf("用户重复注册\n");
        pmsg->action = 11; //重复注册
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else if (regflag == 0)
    {
        //用户未注册,正常注册
        SaveData(pmsg);
        printf("用户注册成功\n");

        pmsg->id = UserRegID(pmsg, cfd);

        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
}

//用户登录
void server_login(Msg *pmsg, int cfd)
{
    printf("用户登录\n");
    printf("用户:%d 登录 密码:%s\n", pmsg->id, pmsg->password);

    //登录判断
    int logflag = -1;
    logflag = LoginSelectDatabase(pmsg, cfd);
    if (logflag == 0)
    {
        //账号不存在
        printf("账号不存在,请注册\n");
        memset(pmsg, 0, sizeof(Msg));
        pmsg->action = 12;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else if (logflag == 1)
    {
        //密码错误
        printf("密码错误,请重试\n");
        memset(pmsg, 0, sizeof(Msg));
        pmsg->action = 13;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else if (logflag == 2)
    {
        //账号id密码正确,登录
        //需要判断是否重复登录
        int RelogFlag = -1;
        RelogFlag = ReLogin(pmsg, cfd);
        if (RelogFlag == 0)
        {
            //用户未登录,正常登录即可
            OnlineUser *new_user = (OnlineUser *)malloc(sizeof(OnlineUser));
            insert_link(new_user, cfd, pmsg);
            // write(cfd, pmsg, sizeof(Msg));
            if (write(cfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
        }
        else if (RelogFlag == 1)
        {
            //用户已登录,无法重复登录
            pmsg->action = 14;
            // write(cfd, pmsg, sizeof(Msg));
            if (write(cfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
        }
    }
    printf("用户id:%d 用户名:%s 登录 密码:%s\n", pmsg->id, pmsg->name, pmsg->password);
}

//私聊
void server_pchat(Msg *pmsg, int cfd)
{
    printf("私聊\n");
    if (strcmp(pmsg->name, pmsg->toname) == 0)
    {
        //私聊对象为自己
        pmsg->action = 18;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        return;
    }

    int to_fd = find_tofd(pmsg->toname);
    if (to_fd == -1)
    {
        printf("用户 %s 的私聊对象 %s 不在线\n", pmsg->name, pmsg->toname);
        pmsg->action = 6;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else
    {
        pmsg->action = 3;
        // write(to_fd, pmsg, sizeof(Msg));
        if (write(to_fd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        save_history(pmsg, cfd);
        //发给私聊对象后,再发给自己
        pmsg->action = 15;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        char result[1024] = {0};
        RC4Decrypt(pmsg->name, pmsg->message, result);
        printf("用户 %s 对 %s 私聊 内容 %s\n", pmsg->name, pmsg->toname, result);
    }
}

void server_group_chat(Msg *pmsg, int cfd)
{
    printf("群聊\n");

    OnlineUser *p = head;
    //对所有在线用户发送消息
    // pmsg->action = 4;
    p = head;
    while (p->next != head)
    {
        p = p->next;
        if (p->cfd == cfd) //跳过自己
        {
            continue;
        }
        // write(p->cfd, pmsg, sizeof(Msg));
        if (write(p->cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }

    char result[1024] = {0};
    RC4Decrypt(pmsg->name, pmsg->message, result);
    printf("%s对所有人说%s\n", pmsg->name, result);
    strcpy(pmsg->toname, "all");
    save_history(pmsg, cfd);

    //给所有人发完后,再给自己
    pmsg->action = 16;
    // write(cfd, pmsg, sizeof(Msg));
    if (write(cfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//私聊,查找在线用户的cfd,不存在返回-1
int find_tofd(char *to_name)
{
    OnlineUser *p = head;
    while (p->next != head)
    {
        p = p->next;
        if (strcmp(p->name, to_name) == 0)
        {
            return p->cfd;
        }
    }
    return -1;
}

//显示所有在线用户
void display_online_user(Msg *pmsg, int cfd)
{
    OnlineUser *p = head;

    //遍历收集所有在线用户姓名
    while (p->next != head)
    {
        p = p->next;
        strcat(pmsg->message, p->name);
        strcat(pmsg->message, " ");
    }
    pmsg->action = 5;
    // write(cfd, pmsg, sizeof(Msg));
    if (write(cfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//判断是否已经登录,未登录0且发送action 17,登录1,同时读取name、管理员、禁言标志位
int online_check(Msg *pmsg, int cfd)
{
    int flag = 0;
    OnlineUser *p = head;

    //将链表中的数据读到pmsg中
    while (p->next != head)
    {
        p = p->next;
        if (p->cfd == cfd)
        {
            strcpy(pmsg->name, p->name);
            pmsg->admin = p->admin;
            pmsg->connfd = p->connfd;
            break;
        }
    }

    p = head;
    while (p->next != head)
    {
        p = p->next;
        if (strcmp(pmsg->name, p->name) == 0)
        {
            flag = 1; //用户在线
        }
    }
    if (flag == 0)
    {
        pmsg->action = 17;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }

    return flag;
}

//管理员禁言用户
void server_forbid(Msg *pmsg, int cfd)
{
    //判断禁言操作者是否为管理员
    if (pmsg->admin == 0)
    {
        printf("用户:%s 不是管理员,没有禁言权限\n", pmsg->name);
        pmsg->action = 28;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        return;
    }

    OnlineUser *p = head;
    // //先找到禁言发起人的name
    //由于调用前又做了在线判断,把信息读取到pmsg中,
    //不需要再找发起人
    // while (p->next != head)
    // {
    //     p = p->next;
    //     if (p->cfd == cfd)
    //     {
    //         strcpy(pmsg->name, p->name);
    //         break;
    //     }
    // }

    // printf("pmsg->name = %s,toname = %s,action = %d\n", pmsg->name, pmsg->toname, pmsg->action);
    //根据被禁言者toname查找cfd
    int to_fd = find_tofd(pmsg->toname);
    if (to_fd == -1)
    {
        printf("用户 %s 的禁言对象 %s 不在线\n", pmsg->name, pmsg->toname);
        pmsg->action = 6;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else
    {
        //先判断禁言对象是否为管理员
        p = head;
        while (p->next != head)
        {
            p = p->next;
            if (p->cfd == to_fd)
            {
                if (p->admin == 1) //禁言对象为管理员
                {
                    pmsg->action = 20;
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                    return;
                }
            }
        }
        //重复禁言判断
        int flag = 0;
        p = head;
        while (p->next != head)
        {
            p = p->next;
            if (p->cfd == to_fd)
            {
                if (p->connfd == 1)
                {
                    //用户已经被禁言,无法重复禁言
                    printf("用户:%s 已经被禁言,无法重复禁言\n", p->name);
                    pmsg->action = 22; //发给自己,无法重复禁言
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                    flag = 1;
                    break;
                }
            }
        }
        if (flag == 0) //用户还没被禁言,可以操作
        {
            p->connfd = 1; //设置禁言
            pmsg->connfd = 1;
            pmsg->action = 19;
            // write(to_fd, pmsg, sizeof(Msg)); //发给被禁者
            if (write(to_fd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
            updataBase(pmsg); //更新数据库
            //发给禁言对象后,再发给自己
            pmsg->action = 21;
            // write(cfd, pmsg, sizeof(Msg));
            if (write(cfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
        }
    }
    printf("管理员:%s 禁言用户:%s\n", pmsg->name, pmsg->toname);
}

//管理员解禁用户
void server_relieve(Msg *pmsg, int cfd)
{
    //判断解禁操作者是否为管理员
    OnlineUser *p = head;
    if (pmsg->admin == 0)
    {
        printf("用户:%s 不是管理员,没有解禁权限\n", pmsg->name);
        pmsg->action = 29;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        return;
    }

    // printf("pmsg->name = %s,toname = %s,action = %d\n", pmsg->name, pmsg->toname, pmsg->action);
    //根据被解禁者toname查找cfd
    int to_fd = find_tofd(pmsg->toname);
    if (to_fd == -1)
    {
        printf("用户 %s 的解禁对象 %s 不在线\n", pmsg->name, pmsg->toname);
        pmsg->action = 6;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else
    {
        //如果解禁对象为管理员
        //管理员不会被禁言,会被提示重复解禁

        //重复解禁判断
        int flag = 0;
        p = head;
        while (p->next != head)
        {
            p = p->next;
            if (p->cfd == to_fd)
            {
                if (p->connfd == 0)
                {
                    //用户没有被禁言,无法重复解禁
                    printf("用户:%s 没有被禁言,无需解禁\n", p->name);
                    pmsg->action = 24; //发给解禁者,无法重复解禁
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                    flag = 1;
                    break;
                }
            }
        }
        if (flag == 0) //用户被禁言,可以解禁
        {
            p->connfd = 0; //设置解除禁言
            pmsg->connfd = 0;
            pmsg->action = 25;
            // write(to_fd, pmsg, sizeof(Msg)); //发给被解禁者
            if (write(to_fd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }

            updataBase(pmsg); //更新数据库
            //发给解禁对象后,再发给自己
            pmsg->action = 23;
            // write(cfd, pmsg, sizeof(Msg));
            if (write(cfd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }
        }
    }
    printf("管理员:%s 解禁用户:%s\n", pmsg->name, pmsg->toname);
}

//用户正常退出
void server_exit(Msg *pmsg, int cfd)
{
    OnlineUser *p = head;
    while (p->next != head)
    {
        p = p->next;
        if (p->cfd == cfd)
        {
            pmsg->action = 7;
            strcpy(pmsg->name, p->name);
            delect_node(p);
            break;
        }
    }
    // write(cfd, pmsg, sizeof(Msg));
    if (write(cfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//管理员踢人
void server_KickOut(Msg *pmsg, int cfd)
{
    //判断踢人操作者是否为管理员
    OnlineUser *p = head;
    if (pmsg->admin == 0)
    {
        printf("用户:%s 不是管理员,没有踢人权限\n", pmsg->name);
        pmsg->action = 30;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        return;
    }

    // printf("pmsg->name = %s,toname = %s,action = %d\n", pmsg->name, pmsg->toname, pmsg->action);
    //根据被踢出者toname查找cfd
    int to_fd = find_tofd(pmsg->toname);
    if (to_fd == -1)
    {
        printf("用户 %s 的踢出对象 %s 不在线\n", pmsg->name, pmsg->toname);
        pmsg->action = 6;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else if (to_fd > 0) //在线用户的cfd大于0
    {
        //用户在线,可以被踢出
        //先判断禁言对象是否为管理员
        p = head;
        while (p->next != head)
        {
            p = p->next;
            if (p->cfd == to_fd)
            {
                if (p->admin == 1) //踢出对象为管理员
                {
                    pmsg->action = 31;
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                    break; // return;
                }
                else
                {
                    pmsg->action = 26;
                    // write(to_fd, pmsg, sizeof(Msg)); //发给被踢出者
                    if (write(to_fd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                    //发给被踢出对象后,再发给自己
                    pmsg->action = 27;
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                    delect_node(p);
                    break;
                }
            }
        }
    }
    else if (to_fd == 0)
    {
        printf("踢人出错\n"); //见鬼了
    }

    printf("管理员:%s 踢出用户:%s\n", pmsg->name, pmsg->toname);
}

//收到发送文件请求
void file_check(Msg *pmsg, int cfd)
{
    printf("发文件请求\n");
    if (strcmp(pmsg->name, pmsg->toname) == 0)
    {
        //发送文件对象为自己
        pmsg->action = 32;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        return;
    }
    // printf("pmsg->name = %s,toname = %s,action = %d\n", pmsg->name, pmsg->toname, pmsg->action);
    int to_fd = find_tofd(pmsg->toname);
    if (to_fd == -1)
    {
        printf("用户 %s 的发送文件对象 %s 不在线\n", pmsg->name, pmsg->toname);
        pmsg->action = 6;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else
    {
        pmsg->action = 33;
        // write(to_fd, pmsg, sizeof(Msg));
        if (write(to_fd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        //发给发文件对象后,再发给自己
        pmsg->action = 34;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
}

//接收方 确认或者拒绝 返回给服务器做处理
void recv_file_request(Msg *pmsg, int cfd)
{
    int to_fd = -1;
    // printf("name = %s,toname = %s\n", pmsg->name, pmsg->toname);
    to_fd = find_tofd(pmsg->toname);
    pmsg->action = 35;
    // write(to_fd, pmsg, sizeof(Msg));
    if (write(to_fd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//密码找回
void server_passwd_recovery(Msg *pmsg, int cfd)
{
    int check = passwd_security(pmsg, cfd);
    if (check == -1)
    {
        //账号找不到(一般没有这种情况)
        pmsg->action = 10;
        // write(cfd, pmsg, sizeof(Msg));
        // return;
    }
    else if (check == 0)
    {
        //密保不通过
        printf("密保不通过\n");
        pmsg->action = 9;
        // write(cfd, pmsg, sizeof(Msg));
    }
    else if (check == 1)
    {
        //密保通过,修改密码
        printf("密保通过\n");
        pmsg->action = 8;
        updataBase(pmsg);
    }
    // write(cfd, pmsg, sizeof(Msg));
    if (write(cfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//确认完毕后发送文件
void server_resend_file(Msg *pmsg, int cfd)
{
    pmsg->action = 36;
    int to_fd = -1;
    to_fd = find_tofd(pmsg->name);
    // write(to_fd, pmsg, sizeof(Msg));
    if (write(to_fd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//查看历史记录
void server_check_history(Msg *pmsg, int cfd)
{
    printf("用户:%s查看历史记录\n", pmsg->name);
    char name[20] = {0};
    strcpy(name, pmsg->name);
    memset(pmsg, 0, sizeof(Msg));
    strcpy(pmsg->name, name);
    get_history_database(pmsg);

    //获取历史记录后发给客户端
    pmsg->action = 37;
    // write(cfd, pmsg, sizeof(Msg));
    if (write(cfd, pmsg, sizeof(Msg)) < 0)
    {
        perror("write data error!");
        exit(1);
    }
}

//接收文件信息，添加连接到gobalconn[]数组，创建填充文件，map到内存
void recv_fileinfo(Msg *pmsg, int sockfd)
{
    //接收文件信息
    char fileinfo_buf[100] = {0};
    int info_len = sizeof(struct file_info);
    memset(fileinfo_buf, 0, info_len);
    int n = 0;
    for (n = 0; n < info_len; n++)
    {
        recv(sockfd, &fileinfo_buf[n], 1, 0);
    }
    struct file_info finfo;
    memcpy(&finfo, fileinfo_buf, info_len);

    printf("------- 文件信息 -------\n");
    printf("文件名 = %s\n文件大小 = %d\n块数 = %d\n标准块大小 = %d\n", finfo.file_name, finfo.file_size, finfo.count, finfo.bs);
    printf("------------------------\n");
}

// 接收客户端1发来的文件
void recv_filedata(int sockfd, int tocfd)
{
    //读取分块头部信息
    int head_len = sizeof(struct file_head);

    struct file_head fhead;
    // read(sockfd, &fhead, head_len);
    if (read(sockfd, &fhead, head_len) < 0)
    {
        perror("read data error!");
        exit(1);
    }

    // int recv_offset = fhead.offset;
    // char *fp = tempbegin + recv_offset;
    char *fp = (char *)malloc(RECVBUF_SIZE);

    //发给客户端2   头部信息
    send(tocfd, &fhead, head_len, 0);

    //接受数据，往map内存写
    int remain_size = fhead.bs; //数据块中待接收数据大小
    int err = fhead.bs % RECVBUF_SIZE;
    int size = 0; //一次recv接受数据大小

#if 1
    while (remain_size > 0)
    {
        if ((size = read(sockfd, fp, RECVBUF_SIZE)) > 0)
        {
            send(tocfd, fp, size, 0); //读一点转一点
            // fp += size;
            remain_size -= size;
            // memset(fp,0,RECVBUF_SIZE);
        }
        if (remain_size <= err)
        {
            size = recv(sockfd, fp, remain_size, 0);
            send(tocfd, fp, size, 0); //读一点转一点
            // fp += size;
            remain_size -= size;
            // memset(fp,0,RECVBUF_SIZE);
        }
    }
    free(fp);
    printf("转发中...\n");

    close(sockfd);
    close(tocfd);
#endif
    return;
}

//接收客户端1的发文件请求和文件信息,判断接收方是否在线
void server_recv_request_fileinfo(Msg *pmsg, int cfd)
{
    //接收文件信息
    char fileinfo_buf[100] = {0};
    int info_len = sizeof(struct file_info);
    // printf("len = %d\n", info_len);
    memset(fileinfo_buf, 0, info_len);
    int n = 0;
    for (n = 0; n < info_len; n++)
    {
        recv(cfd, &fileinfo_buf[n], 1, 0);
    }
    struct file_info finfo;
    memcpy(&finfo, fileinfo_buf, info_len);

    //判断发文件对象是否为自己
    if (strcmp(pmsg->name, pmsg->toname) == 0)
    {
        //发文件对象为自己
        pmsg->action = 18;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        return;
    }
    int to_fd = find_tofd(pmsg->toname);
    if (to_fd == -1)
    {
        printf("用户 %s 的发文件对象 %s 不在线\n", pmsg->name, pmsg->toname);
        pmsg->action = 6;
        // write(cfd, pmsg, sizeof(Msg));
        if (write(cfd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
    }
    else
    {
        //转发给客户端2
        pmsg->action = 40;
        // write(to_fd, pmsg, sizeof(Msg));  //收文件请求
        if (write(to_fd, pmsg, sizeof(Msg)) < 0)
        {
            perror("write data error!");
            exit(1);
        }
        send(to_fd, &finfo, info_len, 0); //文件信息
    }
}