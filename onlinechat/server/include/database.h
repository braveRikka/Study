#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>

#include "tcp_server.h"
#include "md5.h"
#include "RC4.h"

#define DatabaseName "User.db" //用户数据库
#define TABLENAME "user"       //用户表名
#define HISTORYTABLE "History" //历史记录

// ret错误判断,返回0说明没有出错,出错会关闭数据库,退出程序
int RetISOK(int ret, char *errmsg, sqlite3 *db);

//执行sql语句
void DoSqlCmd(char *CMD, char *mode);

//初始化数据库和table(打开并且查看表是否存在)
void InitTable();

//清空表中数据(清除所有注册信息)
void TableDataClean();

//保存1条数据到表中
void SaveData(Msg *pmsg);

//从表中查找数据,做注册判断,无重复账号返回0,有重复账号返回1
int RegistSelectDatabase(Msg *pmsg, int cfd);

//从表中查找数据,做登录判断,找不到账号返回0,只找到账号返回1,账号密码正确返回2且设置name
int LoginSelectDatabase(Msg *pmsg, int cfd);

//按照用户名从数据库获得id,可用于登录,返回-1未注册,其余返回正常id
int UserRegID(Msg *pmsg, int cfd);

//操作后保存到数据库(修改禁言,修改密码)
void updataBase(Msg *pmsg);

//修改密码判断,通过返回1,不通过0,找不到账号返回-1
int passwd_security(Msg *pmsg, int cfd);

//初始化历史记录表
void InitHistoryTable();

//保存历史记录
void save_history(Msg *pmsg, int cfd);

//获取当前时间
void get_time(char *now_time);

//从数据库中获取历史记录
void get_history_database(Msg *pmsg);

