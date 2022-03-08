#include "../include/RC4.h"
// RC4算法对数据的加密和解密

//初始化S盒
void InitSbox(unsigned char sbox[])
{
    for (int i = 0; i < 256; i++)
        sbox[i] = i;
}

//密钥填充256数组
void KeyExpansion(unsigned char key[], char *k, int len)
{
    if (len <= 256)
    {
        for (int i = 0; i < 256; i++)
            key[i] = k[i % len];
    }
    if (len > 256)
    {
        for (int i = 0; i < 256; i++)
            key[i] = k[i];
    }
}

//打乱S盒
void UpsetSbox(unsigned char sbox[], unsigned char key[])
{
    int j = 0;
    unsigned char temp;
    int n;
    for (int i = 0; i < 256; i++)
    {
        n = j + (int)sbox[i] + (int)key[i];
        j = n % 256;
        temp = sbox[i];
        sbox[i] = sbox[j];
        sbox[j] = temp;
    }
}

//加解密数据
void RC4DataProcess(unsigned char sbox[], char *sou, char *res)
{
    int i, j;
    i = 0;
    j = 0;
    char ch = *sou;
    char *p = sou;
    char *pt = res;
    while (ch != '\0')
    {
        i = (i + 1) % 256;
        int temp2 = j + (int)sbox[i];
        j = temp2 % 256;
        unsigned char temp;
        temp = sbox[i];
        sbox[i] = sbox[j];
        sbox[j] = temp;
        int temp1 = (int)sbox[i] + (int)sbox[j];
        int t = temp1 % 256;
        char k = sbox[t];
        char cipherchar = ch ^ k;
        *pt = cipherchar;
        pt++;
        p++;
        ch = *p;
        // printf("%c",*pt);
    }
    // printf("\nres = %s\n",res);
}

//加密总函数 密钥 原文 加密结果
void RC4Encrypt(char *k, char *sou, char *res)
{
    unsigned char key[256] = {0x00};
    unsigned char sbox[256] = {0x00};
    int len = strlen(k);
    KeyExpansion(key, k, len);
    InitSbox(sbox);
    UpsetSbox(sbox, key);
    RC4DataProcess(sbox, sou, res);
}

///解密总函数 密钥 加密后原文 解密结果
void RC4Decrypt(char *k1, char *sou, char *res)
{
    unsigned char key[256] = {0x00};
    unsigned char sbox[256] = {0x00};

    int len = strlen(k1);
    KeyExpansion(key, k1, len);
    InitSbox(sbox);
    UpsetSbox(sbox, key);
    RC4DataProcess(sbox, sou, res);
}
