#include "../include/tcp_server.h"
#include "../include/server_action.h"

pthread_mutex_t sock_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    tempbegin = NULL;

    int listenfd;
    //初始化线程池
    struct threadpool *pool = threadpool_init(20, 100);

    listenfd = tcp_server_create(); //创建套接字

    socklen_t sockaddr_len = sizeof(struct sockaddr);

    // epoll
    static struct epoll_event ev;
    static struct epoll_event events[EPOLL_SIZE];
    int epfd = epoll_create1(0);
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    while (1)
    {
        int events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

        // printf("count = %d\n", events_count);

        for (int i = 0; i < events_count; i++)
        {
            if (events[i].data.fd == listenfd)
            {
                int connfd;
                struct sockaddr_in clientaddr;
                while ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &sockaddr_len)) > 0)
                {
                    // printf("connfd = %d\n", connfd);
                    printf("客户端: ip = %s port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                    threadpool_add_job(pool, thread_work, (void *)connfd);
                    // printf("111\n");
                }
            }
        }
    }
    thread_destory(pool);

    return 0;
}
//线程函数
void *thread_work(void *arg)
{
    pthread_t id = pthread_self();
    pthread_detach(id);

    int cfd = (int)arg;
    Msg *pmsg = (Msg *)malloc(sizeof(Msg));

    while (1)
    {
        memset(pmsg, 0, sizeof(Msg));
        int r_n = read(cfd, pmsg, sizeof(Msg));
        if (r_n == 0)
        {
            // printf("客户端退出\n");
            OnlineUser *p = head;
            while (p->next != head)
            {
                p = p->next;
                if (p->cfd == cfd)
                {
                    delect_node(p);
                    break;
                }
            }
            close(cfd);
            return NULL;
            // break;
            // pthread_exit(NULL);
        }

        switch (pmsg->action)
        {
        case 1:
        {
            //用户注册
            server_regist(pmsg, cfd);

            break;
        }
        case 2:
        {
            //用户登录
            server_login(pmsg, cfd);
            //登录后显示在线用户
            display_online_user(pmsg, cfd);
            break;
        }
        case 3:
        {
            //私聊
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                if (pmsg->connfd == 1) //发起人被禁言
                {
                    pmsg->action = 19;
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                }
                else if (pmsg->connfd == 0) //发起人未被禁言,正常私聊
                {
                    server_pchat(pmsg, cfd);
                }
            }
            break;
        }
        case 4:
        {
            printf("发起群聊\n");
            //群聊
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                // printf("用户已登录\n");
                if (pmsg->connfd == 1) //发起人被禁言
                {
                    // printf("发起人被禁言\n");
                    pmsg->action = 19;
                    // write(cfd, pmsg, sizeof(Msg));
                    if (write(cfd, pmsg, sizeof(Msg)) < 0)
                    {
                        perror("write data error!");
                        exit(1);
                    }
                }
                else if (pmsg->connfd == 0) //发起人未被禁言,正常群聊
                {
                    // printf("发起人未被禁言\n");
                    server_group_chat(pmsg, cfd);
                }
                // printf("action = %d,connfd = %d\n", pmsg->action, pmsg->connfd);
            }
            break;
        }
        case 5:
        {
            //手动显示在线用户
            display_online_user(pmsg, cfd);
            break;
        }
        case 6:
        {
            //密码找回
            server_passwd_recovery(pmsg, cfd);
            break;
        }
        case 7:
        {
            //禁言
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                server_forbid(pmsg, cfd);
            }
            break;
        }
        case 8:
        {
            //解禁
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                server_relieve(pmsg, cfd);
            }
            break;
        }
        case 9:
        {
            //踢人
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                server_KickOut(pmsg, cfd);
            }
            break;
        }
        case 10:
        {
            //请求查看在线用户
            display_online_user(pmsg, cfd);
            break;
        }
        case 11:
        {
            //用户正常退出
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                printf("用户:%s 正常退出\n", pmsg->name);
                server_exit(pmsg, cfd);
            }
            break;
        }
        case 12:
        {
            //收到发送文件请求
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                file_check(pmsg, cfd);
            }
            break;
        }
        case 13:
        {
            //收到接收文件方的回应
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                recv_file_request(pmsg, cfd);
            }
            break;
        }
        case 14:
        {
            //读取到发文件者(toname)的文件,调用函数发给接收方(name)
            server_resend_file(pmsg, cfd);
            break;
        }
        case 15:
        {
            //查看历史记录
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                server_check_history(pmsg, cfd);
            }
            break;
        }
        case 16:
        {
            // 收到传大文件的请求
            if (online_check(pmsg, cfd) == 1) //先判断发起人是否在线
            {
                //接收客户端1的发文件请求和文件信息,判断接收方是否在线
                server_recv_request_fileinfo(pmsg, cfd);
            }

            break;
        }
        case 17:
        {
            //客户端发来文件块     改进后原功能报废 这条空了
            printf("客户端发来action = 17\n");

            break;
        }
        case 18:
        {
            //客户端2收文件请求 接受/拒绝

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

            // int tmpfd = open(finfo.file_name, O_RDWR | O_CREAT, 0655);
            // fchmod(tmpfd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            // ftruncate(tmpfd, finfo.file_size);
            // tempbegin = (char *)mmap(NULL, finfo.file_size, PROT_WRITE | PROT_READ, MAP_SHARED, tmpfd, 0);
            // close(tmpfd);

            int to_fd = -1;
            to_fd = find_tofd(pmsg->name);
            // printf("name = %s toname = %s\n", pmsg->name, pmsg->toname);
            pmsg->action = 41;
            // printf("recv quest = %d\n", pmsg->file_request);
            // write(to_fd, pmsg, sizeof(Msg));
            if (write(to_fd, pmsg, sizeof(Msg)) < 0)
            {
                perror("write data error!");
                exit(1);
            }

            send(to_fd, &finfo, info_len, 0); //文件信息
            if (pmsg->file_request == 0)
            {
                // freeid
                char id_buf[4] = {0};
                int n = 0;
                for (n = 0; n < 4; n++)
                {
                    // read(cfd, &id_buf[n], 1);
                    if (read(cfd, &id_buf[n], 1) < 0)
                    {
                        perror("read data error!");
                        exit(1);
                    }
                }
                // int freeid = *((int *)id_buf);
                // printf("freeid = %d\n", freeid);

                //发freeid
                send(to_fd, id_buf, 4, 0);
            }
            break;
        }
        case 19:
        {
            // 从套接字数组读取套接字
            int tocfd = -1;
            pthread_mutex_lock(&sock_mutex);
            for (int i = 0; i < 20; i++) //后面改成while循环
            {
                if (CLI_sfd[i].used == 1) //已经被使用
                {
                    if (strcmp(CLI_sfd[i].clientName, pmsg->toname) == 0)
                    {
                        tocfd = CLI_sfd[i].client_1_sfd; //找到对应套接口
                        CLI_sfd[i].used = 0;             //改成未被使用
                        // printf("CLI_sfd[%d].client_1_sfd = %d , used = %d\n", i, CLI_sfd[i].client_1_sfd, CLI_sfd[i].used);
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&sock_mutex);
            // printf("客户端1线程连接\n");
            // printf("tocfd = %d\n", tocfd);
            // 接收客户端1发来的文件
            recv_filedata(cfd, tocfd); // cfd是1的  tocfd是2的
            break;
        }
        case 20:
        {
            //客户端2多线程连接服务器
            //一连上就把套接口放到数组中

            pthread_mutex_lock(&sock_mutex);
            for (int i = 0; i < 20; i++) //后面改成while循环
            {
                if (CLI_sfd[i].used == 0) //未被使用
                {
                    strcpy(CLI_sfd[i].clientName, pmsg->toname);
                    CLI_sfd[i].client_1_sfd = cfd;
                    CLI_sfd[i].used = 1; //改成被使用
                    // printf("客户端2线程连接 cfd = %d\n", cfd);
                    break;
                }
            }
            pthread_mutex_unlock(&sock_mutex);
            break;
        }
            // case 1:
            // {

            //     break;
            // }

        default:
            break;
        }
    }
}
