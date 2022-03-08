#pragma once
// RC4算法对数据的加密和解密
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//加解密密钥k k1 长度25

//函数声明
void InitSbox(unsigned char sbox[]);
void KeyExpansion(unsigned char key[], char *k, int len);
void UpsetSbox(unsigned char sbox[], unsigned char key[]);
void RC4DataProcess(unsigned char sbox[], char *p, char *pt);

//加密总函数 密钥 原文 加密结果
void RC4Encrypt(char *k, char *sou, char *res);
///解密总函数 密钥 加密后原文 解密结果
void RC4Decrypt(char *k1, char *sou, char *res);
