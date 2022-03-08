#pragma once
#include "../include/tcp_server.h"
// #include "tpool.h"
#include "../include/database.h"
#include "../include/thread_pool.h"


void server_regist(Msg *pmsg, int cfd); //用户注册

void server_login(Msg *pmsg, int cfd); //用户登录

//私聊
void server_pchat(Msg *pmsg, int cfd);

//私聊,查找在线用户的cfd,不存在返回-1
int find_tofd(char *to_name);

//群聊
void server_group_chat(Msg *pmsg, int cfd);

//显示所有在线用户
void display_online_user(Msg *pmsg, int cfd);

//判断是否已经登录,未登录0且发送action 17,登录1,同时读取管理员、禁言标志位
int online_check(Msg *pmsg, int cfd);

//找回密码
void pwd_recovery(Msg *pmsg, int cfd); //还没写

//管理员禁言用户
void server_forbid(Msg *pmsg, int cfd);

//管理员解禁用户
void server_relieve(Msg *pmsg, int cfd);

//用户正常退出
void server_exit(Msg *pmsg, int cfd);

//管理员踢人
void server_KickOut(Msg *pmsg, int cfd);

//密码找回
void server_passwd_recovery(Msg *pmsg, int cfd);

//修改数据(禁言、解禁、修改密码)

//收到发送文件请求
void file_check(Msg *pmsg, int cfd);

//接收方 确认或者拒绝 返回给服务器做处理
void recv_file_request(Msg *pmsg, int cfd);

//确认完毕后发送文件
void server_resend_file(Msg *pmsg, int cfd);

//查看历史记录
void server_check_history(Msg *pmsg, int cfd);

//接收文件信息，添加连接到gobalconn[]数组，创建填充文件，map到内存
void recv_fileinfo(Msg *pmsg, int sockfd);


//接收客户端1的发文件请求和文件信息,判断接收方是否在线
void server_recv_request_fileinfo(Msg *pmsg, int cfd);

// 接收客户端1发来的文件
void recv_filedata(int sockfd,int tocfd);

