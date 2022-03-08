#include "tcp_client.h"
#include "msg.h"
#include "RC4.h"

void *choice_menu(void *arg)
{
    pthread_t id = pthread_self();
    pthread_detach(id);

    int sockfd = (int)arg;
    Msg *pmsg = (Msg *)malloc(sizeof(Msg));
    char cmd[1024] = {0};
    // menu();
    printf("cmd: reg log chat group exit\n");
    printf("forbid relieve kick file help/menu\n");

    while (1)
    {
        sleep(1);
        printf("请输入cmd:\n");
        scanf("%s", cmd);
        getchar(); //吞掉输入错误的操作

        if ((strcmp(cmd, "help") == 0) || (strcmp(cmd, "menu") == 0))
        {
            menu();
        }
        if (strcmp(cmd, "reg") == 0)
        {
            regist(pmsg, sockfd);
        }
        if (strcmp(cmd, "log") == 0)
        {
            login(pmsg, sockfd);
        }
        if (strcmp(cmd, "chat") == 0)
        {
            pricate_chat(pmsg, sockfd);
        }
        if (strcmp(cmd, "group") == 0)
        {
            group_chat(pmsg, sockfd);
        }
        if (strcmp(cmd, "user") == 0)
        {
            display(pmsg, sockfd);
        }
        if (strcmp(cmd, "forbid") == 0)
        {
            forbid(pmsg, sockfd);
        }
        if (strcmp(cmd, "relieve") == 0)
        {
            relieve(pmsg, sockfd);
        }
        if (strcmp(cmd, "exit") == 0)
        {
            user_exit(pmsg, sockfd);
        }
        if (strcmp(cmd, "kick") == 0)
        {
            kick_out(pmsg, sockfd);
        }
        if (strcmp(cmd, "file") == 0)
        {
            sfile_check(pmsg, sockfd);
        }
        if (strcmp(cmd, "forget") == 0)
        {
            passwd_recovery(pmsg, sockfd);
        }
        if (strcmp(cmd, "name") == 0)
        {
            my_name(); //显示当前用户
        }
        if (strcmp(cmd, "history") == 0)
        {
            cheak_history(pmsg, sockfd);
        }
        if (strcmp(cmd, "send") == 0)
        {
            //调用传大文件函数
            SendBigFile(pmsg, sockfd); //传大文件
        }
        if (strcmp(cmd, "close") == 0)
        {
            if (strcmp(Client_Name, "") != 0)
            {
                user_exit(pmsg, sockfd);
            }
            close(sockfd);
            printf("已关闭\n");
            exit(1);
        }
        if (actionFlag == 40)
        {
            if ((strcmp(cmd,"y")==0) || (strcmp(cmd,"Y") == 0))
            {
                c2_recv = 1;
            }
            else if ((strcmp(cmd,"n")==0) || (strcmp(cmd,"N") == 0))
            {
                c2_recv = 2;
            }
            
            sem_post(&sems[0]);
        }
    }
}

int main(int argc, char **argv)
{
    //用于本地判断
    memset(Client_Name, 0, 20); //用户名
    Client_Id = -1;             //登录id (1起始)
    connfd = 0;                 //被禁言标志0/1
    admin = 0;                  //普通用户0 管理员1
    file_to_cfd = 0;            //传文件目标cfd
    actionFlag = 0;

    c2_recv = 0;

    memset(Client_FileName, 0, 30);
    fd = 0;
    last_bs = 0;
    memset(id_buf, 0, 4);

    // freeid = 0;

    if (argc != 3)
    {
        printf("Please input server ip and port!\n");
        exit(1);
    }

    strcpy(server_ip, argv[1]);
    port = atoi(argv[2]);
    int sockfd = Client_init(argv[1]);

    sem_init(&sems[0], 0, 0);

    pthread_create(&id, NULL, thread_read, (void *)sockfd);
    pthread_create(&id2, NULL, choice_menu, (void *)sockfd);

    pause();

    return 0;
}

//线程处理函数
void *thread_read(void *arg)
{
    pthread_t id = pthread_self();
    pthread_detach(id);

    int cfd = (int)arg;
    Msg *pmsg = (Msg *)malloc(sizeof(Msg));

    while (1)
    {
        memset(pmsg, 0, sizeof(Msg));
        int r_n = read(cfd, pmsg, sizeof(Msg));
        actionFlag = pmsg->action;

        if (r_n == 0)
        {
            printf("server is exit!\n");
            pthread_exit(NULL);
        }

        switch (pmsg->action)
        {
        case 1:
        {
            //注册
            printf("注册成功\n");
            printf("id = %d\n请使用id登录\n", pmsg->id);
            break;
        }
        case 2:
        {
            printf("登录成功\n");
            strcpy(Client_Name, pmsg->name);
            Client_Id = pmsg->id;
            admin = pmsg->admin;
            connfd = pmsg->connfd;

            // printf("name = %s id = %d\nadmin = %d connfd = %d\n", Client_Name, Client_Id, admin, connfd);
            break;
        }
        case 3:
        {
            char result[1024] = {0};
            RC4Decrypt(pmsg->name, pmsg->message, result);
            printf("接收%s的消息:%s\n", pmsg->name, result);
            break;
        }
        case 4:
        {
            //群聊
            char result[1024] = {0};
            RC4Decrypt(pmsg->name, pmsg->message, result);
            printf("%s对所有人说%s\n", pmsg->name, result);
            break;
        }
        case 5:
        {
            //查看在线用户
            printf("在线用户\n%s\n", pmsg->message);
            break;
        }
        case 6:
        {
            //私聊、禁言、解禁、踢出、发文件对象不在线
            printf("对象:%s 不在线\n", pmsg->toname);
            break;
        }
        case 7:
        {
            printf("用户:%s 您已退出\n", pmsg->name);
            //清空数据
            memset(Client_Name, 0, sizeof(Client_Name));
            Client_Id = UNLOGIN;
            connfd = UNFORBID;
            admin = UNADMIN;
            memset(pmsg, 0, sizeof(Msg));

            break;
        }
        case 8:
        {
            //修改密码成功
            printf("修改密码成功\n");
            break;
        }
        case 9:
        {
            //密保不通过,修改密码失败
            printf("密保错误,修改找回密码失败\n");
            break;
        }
        case 10:
        {
            //账号不存在
            printf("账号不存在\n"); //见鬼了
            break;
        }
        case 11:
        {
            printf("该用户已注册,请登录\n");
            break;
        }
        case 12:
        {
            printf("账号不存在,请注册\n");
            break;
        }
        case 13:
        {
            printf("密码错误,请重试\n");
            break;
        }
        case 14:
        {
            printf("用户已登录,请勿重复登录\n");
            break;
        }
        case 15:
        {
            char result[1024] = {0};
            RC4Decrypt(Client_Name, pmsg->message, result);
            printf("你对%s说%s\n", pmsg->toname, result);
            break;
        }
        case 16:
        {
            char result[1024] = {0};
            RC4Decrypt(Client_Name, pmsg->message, result);
            printf("你对所有人说%s\n", result);
            break;
        }
        case 17:
        {
            //发起人未登录
            printf("您未登录,请先登录\n");
            break;
        }
        case 18:
        {
            //私聊 发文件对象为自己
            printf("对象不能是自己\n");
            break;
        }
        case 19:
        {
            //被禁言时  或者被禁言情况下发起聊天
            printf("您已被禁言,操作者:%s\n", pmsg->name);
            connfd = FORBID;
            break;
        }
        case 20:
        {
            //禁言对象为管理员(包括自己)
            printf("无法禁言管理员\n");
            break;
        }
        case 21:
        {
            //管理员禁言用户
            printf("您是管理员:%s\n", pmsg->name);
            printf("您已禁言用户:%s\n", pmsg->toname);
            break;
        }
        case 22:
        {
            //管理员重复禁言用户
            printf("您是管理员:%s\n", pmsg->name);
            printf("用户:%s已被禁言\n无法重复禁言\n", pmsg->toname);
            break;
        }
        case 23:
        {
            printf("您是管理员:%s\n", pmsg->name);
            printf("您已解禁用户:%s\n", pmsg->toname);
            break;
        }
        case 24:
        {
            //管理员重复解禁用户
            printf("您是管理员:%s\n", pmsg->name);
            printf("用户:%s没有被禁言\n无法重复接触禁言\n", pmsg->toname);
            break;
        }
        case 25:
        {
            printf("您已被解除禁言,操作者:%s\n", pmsg->name);
            connfd = UNFORBID;
            break;
        }
        case 26:
        {
            printf("您已被踢出,操作者:%s\n", pmsg->name);
            //清空数据
            memset(Client_Name, 0, sizeof(Client_Name));
            Client_Id = UNLOGIN;
            connfd = UNFORBID;
            admin = UNADMIN;
            memset(pmsg, 0, sizeof(Msg));
            break;
        }
        case 27:
        {
            printf("您是管理员:%s\n", pmsg->name);
            printf("您已踢出用户:%s\n", pmsg->toname);
            break;
        }
        case 28:
        {
            printf("您不是管理员,没有禁言权限\n");
            break;
        }
        case 29:
        {
            printf("您不是管理员,没有解禁权限\n");
            break;
        }
        case 30:
        {
            printf("您不是管理员,没有踢人权限\n");
            break;
        }
        case 31:
        {
            printf("您无法踢出管理员\n");
            break;
        }
        case 32:
        {
            //发送文件对象为自己
            printf("发送文件对象不能是自己\n");
            break;
        }
        case 33:
        {
            //用户收到是否接收文件请求
            actionFlag = pmsg->action;
            printf("用户:%s 向您发送文件,是否接收(y/n)\n", pmsg->name);
            char temp[20] = {0};

            strcpy(pmsg->toname, pmsg->name);

            char ch;
            scanf("%c", &ch);
            getchar();

            if ((ch == 'y') || (ch == 'Y'))
            {
                pmsg->file_request = 0; //同意接收
            }
            else if ((ch == 'n') || (ch == 'N'))
            {
                pmsg->file_request = 1; //拒绝接收
            }
            //接收方 确认或者拒绝 返回服务器
            pmsg->action = 18;
            write(cfd, pmsg, sizeof(Msg));
            break;
        }
        case 34:
        {
            //发文件用户等待接收方确认
            printf("用户:%s 在线,等待对方确认\n", pmsg->toname);
            break;
        }
        case 35:
        {
            if (pmsg->file_request == 1)
            {
                //用户拒绝接收文件
                printf("用户:%s 拒绝接收文件\n", pmsg->name);
            }
            else if (pmsg->file_request == 0)
            {
                //用户同意接收文件
                printf("用户:%s 同意接收文件\n", pmsg->name);
            }
            break;
        }
        case 36:
        {
            int to_fd = open(pmsg->file_path, O_RDWR | O_CREAT, 0655);
            write(to_fd, pmsg->file, 1024);
            close(to_fd);
            break;
        }
        case 37:
        {
            //接收历史消息
            printf("历史记录:\n%s\n", pmsg->file);
            break;
        }
        case 38:
        {
            // printf("接收服务器分配ID,文件切块,创建多线程发送文件块\n");

            //发送方
            //接收服务器发来的文件分块信息
            //接收服务器分配ID,文件切块,创建多线程发送文件块
            FileIntend(pmsg, cfd);
            break;
        }
        case 40:
        {
            //客户端2接收到收文件的请求 和 文件信息
            actionFlag = 40;
            client_2_recv_check(pmsg, cfd);
            actionFlag = 0;
            break;
        }
        case 41:
        {
            //客户端1判断客户端2 同意或者拒绝

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

            if (pmsg->file_request == 1)
            {
                printf("用户%s拒绝接收文件\n", pmsg->toname);
            }
            else
            {
                printf("用户%s同意接收文件\n", pmsg->toname);
                // freeid
                // char id_buf[4] = {0};
                int n = 0;
                for (n = 0; n < 4; n++)
                {
                    read(cfd, &id_buf[n], 1);
                }
                freeid = *((int *)id_buf);
                // printf("freeid = %d\n", freeid);
                //调用发文件
                FileIntend(pmsg, cfd);
            }

            break;
        }
            // case 37:
            // {
            //     //暂未使用
            //     break;
            // }

        default:
            break;
        }
    }
}

//初始化客户端,返回sockfd
int Client_init(char *ip)
{
    int sockfd;
    struct sockaddr_in s_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("scoket error!");
        exit(1);
    }

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);          //服务器的端口号
    s_addr.sin_addr.s_addr = inet_addr(ip); //服务器ip地址

    if (connect(sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr)) != 0)
    {
        perror("connect error!");
        exit(1);
    }
    return sockfd;
}