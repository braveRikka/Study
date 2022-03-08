#include "../include/database.h"

// ret错误判断,返回0说明没有出错,出错会关闭数据库,退出程序
int RetISOK(int ret, char *errmsg, sqlite3 *db)
{
    if (ret != SQLITE_OK)
    {
        printf("%s:%s\n", errmsg, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
        return -1;
    }
    return 0;
}

//执行sql语句
void DoSqlCmd(char *CMD, char *mode)
{
    int ret = -1;
    char *errmsg = NULL;
    sqlite3 *db;
    ret = sqlite3_open(DatabaseName, &db);
    RetISOK(ret, "open", db);
    ret = sqlite3_exec(db, CMD, NULL, NULL, &errmsg);
    RetISOK(ret, mode, db);
    sqlite3_close(db);
}

//初始化数据库和table(打开并且查看表是否存在)
void InitTable()
{
    char SqlCMD[256] = {0};
    sprintf(SqlCMD, "create table if not exists '%s'(id integer PRIMARY KEY AUTOINCREMENT UNIQUE,name text UNIQUE,password text,qus text,ans text,admin integer,connfd integer);",
            TABLENAME);
    //自增唯一id,唯一用户名,密码,密保问题,密保答案,管理员标志,禁言标志

    // sprintf(SqlCMD, "create table if not exists '%s'(id integer PRIMARY KEY AUTOINCREMENT UNIQUE,name text UNIQUE,password text);", TABLENAME);
    //自增唯一id,唯一用户名,密码

    DoSqlCmd(SqlCMD, "Init"); //执行数据库语句
}

//清空表中数据(清除所有注册信息)
void TableDataClean()
{
    char SqlCMD[256] = {0};
    sprintf(SqlCMD, "delete from '%s';", TABLENAME);
    DoSqlCmd(SqlCMD, "clean table");
}

//保存1条数据到表中
void SaveData(Msg *pmsg)
{
    char SqlCMD[256] = {0};
    // int count = 0;

    memset(SqlCMD, 0, sizeof(SqlCMD));
    char t_passwd[64] = {0}; //临时存放加密后密码
    char t_question[64] = {0};
    char t_answer[64] = {0};
    //密码、密保、答案 加密
    Md5Encrypt(pmsg->password, t_passwd);
    Md5Encrypt(pmsg->question, t_question);
    Md5Encrypt(pmsg->answer, t_answer);

    sprintf(SqlCMD, "insert into %s values(NULL,'%s','%s','%s','%s',%d,%d);", TABLENAME, pmsg->name, t_passwd, t_question, t_answer, pmsg->admin, pmsg->connfd);
    // printf("cmd = %s\n", SqlCMD);
    DoSqlCmd(SqlCMD, "Insert");
    // count++;

    // if (count != 0)
    // {
    //     printf("成功写入%d条数据\n", count);
    // }
}

//从表中查找数据,做注册判断
int RegistSelectDatabase(Msg *pmsg, int cfd)
{
    int RegFlag = 0; //重复返回1,不重复返回0
    sqlite3 *db;
    char sql[128] = {0};
    int ret = -1;
    // int ncolumn;

    ret = sqlite3_open(DatabaseName, &db);

    sprintf(sql, "select * from %s;", TABLENAME);

    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    ret = sqlite3_step(stmt);
    // ncolumn = sqlite3_column_count(stmt);
    sqlite3_column_count(stmt);
    while (ret == SQLITE_ROW)
    {

        // for (int i = 0; i < ncolumn; i++)
        // {
        //     printf("%s | ", sqlite3_column_text(stmt, i));
        // }
        // printf("\n");

        // ID    整型，字符串需要转换
        // char sqlID[20] = {0};
        // strcpy(sqlID, (char *)sqlite3_column_text(stmt, 0));
        // p->ID = atoi(sqlID);
        // p->ID = (int)(*sqlite3_column_text(stmt, 0));
        // name  字符串，strcpy
        char name[20] = {0};
        strcpy(name, (char *)sqlite3_column_text(stmt, 1));

        if (strcmp(name, pmsg->name) == 0)
        {
            //找到相同的用户,重复注册
            RegFlag = 1;

            //跳出循环
            ret = sqlite3_step(stmt);
            break;
        }

        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);

    return RegFlag;
}

//从表中查找数据,做登录判断,找不到账号返回0,只找到账号返回1,账号密码正确返回2
int LoginSelectDatabase(Msg *pmsg, int cfd)
{
    int LogFlag = 0; //找不到账号返回0,只找到账号返回1,账号密码正确返回2
    sqlite3 *db;
    char sql[128] = {0};
    int ret = -1;
    // int ncolumn;

    ret = sqlite3_open(DatabaseName, &db);

    sprintf(sql, "select * from %s;", TABLENAME);

    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    ret = sqlite3_step(stmt);
    // ncolumn = sqlite3_column_count(stmt);
    sqlite3_column_count(stmt);

    while (ret == SQLITE_ROW)
    {
        int id = atoi((char *)sqlite3_column_text(stmt, 0));
        if (id == pmsg->id)
        {
            //找到相同的用户    ID
            LogFlag = 1;
            char encryption[64] = {0}; //临时存放加密后密码
            //密码加密
            Md5Encrypt(pmsg->password, encryption);
            if (strcmp((char *)sqlite3_column_text(stmt, 2), encryption) == 0)
            {
                // ID对应密码正确
                LogFlag = 2;
                //把1用户名,3密保,4答案,5管理员,6禁言,设置到pmsg后跳出循环
                //由于添加了加密功能,不再设置3密保和4答案
                strcpy(pmsg->name, (char *)sqlite3_column_text(stmt, 1));
                // strcpy(pmsg->question, (char *)sqlite3_column_text(stmt, 3));
                // strcpy(pmsg->answer, (char *)sqlite3_column_text(stmt, 4));
                //管理员标志位
                char sqlAdmin[20] = {0};
                strcpy(sqlAdmin, (char *)sqlite3_column_text(stmt, 5));
                pmsg->admin = atoi(sqlAdmin);
                //禁言标志位
                char sqlConnfd[20] = {0};
                strcpy(sqlConnfd, (char *)sqlite3_column_text(stmt, 6));
                pmsg->connfd = atoi(sqlConnfd);

                ret = sqlite3_step(stmt);
                break;
            }
        }
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);

    return LogFlag;
}

//按照用户名从数据库获得id,可用于登录,返回-1未注册,其余返回正常id
int UserRegID(Msg *pmsg, int cfd)
{
    int RegFlag = -1;
    sqlite3 *db;
    char sql[128] = {0};
    int ret = -1;

    ret = sqlite3_open(DatabaseName, &db);

    sprintf(sql, "select * from %s;", TABLENAME);

    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    ret = sqlite3_step(stmt);

    sqlite3_column_count(stmt);
    while (ret == SQLITE_ROW)
    {
        // ID    整型，字符串需要转换
        // char sqlID[20] = {0};
        // strcpy(sqlID, (char *)sqlite3_column_text(stmt, 0));
        // p->ID = atoi(sqlID);
        // p->ID = (int)(*sqlite3_column_text(stmt, 0));
        // name  字符串，strcpy

        if (strcmp((char *)sqlite3_column_text(stmt, 1), pmsg->name) == 0)
        {
            //找到相同的用户,获取id
            char sqlID[20] = {0};
            strcpy(sqlID, (char *)sqlite3_column_text(stmt, 0));
            RegFlag = atoi(sqlID);

            //跳出循环
            ret = sqlite3_step(stmt);
            break;
        }

        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);

    return RegFlag;
}

//从数据库中修改1条数据
void UpdataDB(Msg *pmsg, int mode)
{
    char SqlCMD[256] = {0};
    // int count = 0;

    if (mode == 1) //修改密码
    {
        memset(SqlCMD, 0, sizeof(SqlCMD));
        sprintf(SqlCMD, "update %s set password = '%s' where name = '%s' and ;", TABLENAME, pmsg->password, pmsg->name);
    }
    else if (mode == 2) //修改密保问题
    {
    }

    // printf("cmd = %s\n", SqlCMD);
    DoSqlCmd(SqlCMD, "updata");
    // count++;

    // if (count != 0)
    // {
    //     printf("成功修改%d条数据\n", count);
    // }
}

//操作后保存到数据库(修改禁言,修改密码)
void updataBase(Msg *pmsg)
{
    char SqlCMD[256] = {0};
    memset(SqlCMD, 0, sizeof(SqlCMD));
    if (pmsg->action == 19) //禁言 name操作者 toname被禁言 connfd = 1
    {
        sprintf(SqlCMD, "update %s set connfd = %d where name = '%s';", TABLENAME, pmsg->connfd, pmsg->toname);
    }
    else if (pmsg->action == 25) //解禁
    {
        sprintf(SqlCMD, "update %s set connfd = %d where name = '%s';", TABLENAME, pmsg->connfd, pmsg->toname);
    }
    else if (pmsg->action == 8) //修改密码成功
    {
        char t_passwd[64] = {0};
        Md5Encrypt(pmsg->password, t_passwd);
        sprintf(SqlCMD, "update %s set password = '%s' where name = '%s';", TABLENAME, t_passwd, pmsg->name);
    }

    // printf("sql = %s\n",SqlCMD);
    DoSqlCmd(SqlCMD, "update");
}

//修改密码判断,通过返回1,不通过0,找不到账号返回-1
int passwd_security(Msg *pmsg, int cfd)
{
    int flag = -1;
    sqlite3 *db;
    char sql[128] = {0};
    int ret = -1;
    ret = sqlite3_open(DatabaseName, &db);
    sprintf(sql, "select * from %s;", TABLENAME);
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    ret = sqlite3_step(stmt);
    sqlite3_column_count(stmt);
    while (ret == SQLITE_ROW)
    {
        if (strcmp((char *)sqlite3_column_text(stmt, 1), pmsg->name) == 0)
        {
            //找到相同的用户,获取id
            flag = 0;

            char sqlID[20] = {0};
            strcpy(sqlID, (char *)sqlite3_column_text(stmt, 0));
            pmsg->id = atoi(sqlID);

            //再判断密保是否通过

            char t_question[64] = {0};
            char t_answer[64] = {0};
            //密保、答案 加密
            Md5Encrypt(pmsg->question, t_question);
            Md5Encrypt(pmsg->answer, t_answer);

            if ((strcmp((char *)sqlite3_column_text(stmt, 3), t_question) == 0) &&
                (strcmp((char *)sqlite3_column_text(stmt, 4), t_answer) == 0))
            {
                //对应密保通过
                flag = 1;
                ret = sqlite3_step(stmt);
                break;
            }
        }
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return flag;
}

//初始化历史记录表
void InitHistoryTable()
{
    char SqlCMD[256] = {0};
    sprintf(SqlCMD, "create table if not exists '%s'(fromname text,toname text,message text,time text);",
            HISTORYTABLE);
    //发送人,接受者,消息内容

    DoSqlCmd(SqlCMD, "history"); //执行数据库语句
}

//保存历史记录
void save_history(Msg *pmsg, int cfd)
{
    char SqlCMD[2048] = {0};
    // int count = 0;
    char now_time[32] = {0};
    get_time(now_time);
    memset(SqlCMD, 0, sizeof(SqlCMD));

    //客户端发过来就是加密的(但是这么做总觉得数据不够安全)
    // char res_message[1024] = {0};
    // RC4Encrypt(pmsg->name, pmsg->message, res_message);

    sprintf(SqlCMD, "insert into %s values('%s','%s','%s','%s');",
            HISTORYTABLE, pmsg->name, pmsg->toname, pmsg->message, now_time);
    // printf("Sqlcmd = %s\n", SqlCMD);
    DoSqlCmd(SqlCMD, "Insert");
}

//获取当前时间
void get_time(char *now_time)
{
    time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now); // get date
    // printf("start datetime: %d-%d-%d %d:%d:%d\n", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

    memset(now_time, 0, 32);
    sprintf(now_time, "%d-%d-%d %d:%d:%d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    // strcpy(now_time,"2022-2-22 14:45:22");
    // printf("len = %d\n",strlen(now_time));
    // printf("%s\n", now_time);
}

//从数据库中获取历史记录
void get_history_database(Msg *pmsg)
{
    // int flag = 0; //找到为0,找到为1
    sqlite3 *db;
    char sql[128] = {0};
    int ret = -1;

    ret = sqlite3_open(DatabaseName, &db);
    sprintf(sql, "select * from %s where fromname = '%s' or toname = '%s' or toname = 'all';",
            HISTORYTABLE, pmsg->name, pmsg->name);
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    ret = sqlite3_step(stmt);
    // ncolumn = sqlite3_column_count(stmt);
    while (ret == SQLITE_ROW)
    {
        strcat(pmsg->file, (char *)sqlite3_column_text(stmt, 3)); //时间
        strcat(pmsg->file, " ");
        strcat(pmsg->file, (char *)sqlite3_column_text(stmt, 0)); //聊天发起方
        strcat(pmsg->file, " 对 ");

        char recname[25] = {0};
        memcpy(recname, (char *)sqlite3_column_text(stmt, 1), 25);
        if (strcmp("all", recname) == 0) //接收方
        {
            strcat(pmsg->file, "所有人");
        }
        else
        {
            strcat(pmsg->file, recname);
        }
        strcat(pmsg->file, " 说 ");

        char result[1024] = {0};
        char fname[25] = {0};
        char tname[25] = {0};
        memcpy(fname, (char *)sqlite3_column_text(stmt, 0), 25);
        memcpy(tname, (char *)sqlite3_column_text(stmt, 1), 25);
        if ((memcmp(fname, pmsg->name, 25) == 0) || (memcmp(tname, "all", 3) == 0))
        {
            //发起方密钥
            printf("fname = %s\n", fname);
            RC4Decrypt(fname, (char *)sqlite3_column_text(stmt, 2), result);
            printf("result = %s\n", result);
        }
        else if (strcmp(tname, pmsg->name) == 0)
        {
            //接收方密钥
            printf("tname = %s\n", tname);
            RC4Decrypt(tname, (char *)sqlite3_column_text(stmt, 2), result);
            printf("result = %s\n", result);
        }
        // else if (strcmp(tname, "all") == 0)
        // {
        //     //发送方密钥
        //     RC4Decrypt(fname, (char *)sqlite3_column_text(stmt, 2), result);
        // }
        strcat(pmsg->file, result);
        strcat(pmsg->file, "\n");
        ret = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}