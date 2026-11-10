#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

long app_common_getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec*1000 + tv.tv_usec/1000;
    // tv_sec: 秒  tv_usec: 微秒
}

/**
 * [abcd] => "61626364"
 *
 * 注意：必须先把 char 转成 unsigned char 再交给 %02X。
 * char 是否有符号由编译器决定（ARM/x86 默认有符号），0x80 以上的字节
 * 会被符号扩展成 0xFFFFFF80，sprintf 按 %02X 会输出 8 个字符，
 * 而缓冲区只按每字节 2 个字符分配 —— 直接堆溢出。
 * 蓝牙链路上的数据多是 ASCII（<0x80）所以从来不会触发；CAN 数据是
 * 任意 0x00~0xFF，一定会踩到。
 */
char *app_common_chars2Hexstr(char *chars, int chars_len)
{
    if (chars == NULL || chars_len < 0)
    {
        return NULL;
    }
    // 申请16进制字符串空间
    char *hexstr = (char *)malloc(chars_len*2 + 1);
    if (hexstr == NULL)
    {
        return NULL;
    }
    // 遍历chars中的每个字符，将字符转换为二位16进制字符保存到hexstr中
    for (int i = 0; i < chars_len; i++)
    {
        sprintf(hexstr + i*2, "%02X", (unsigned char)chars[i]);
    }
    // 向hexstr中添加结束符
    hexstr[chars_len*2] = '\0';

    // 返回hexstr
    return hexstr;
}

/**
 * "61626364" => [abcd]
 *
 * 注意：sscanf 的 %02X 需要 unsigned int* 作为目标，而这里要写的是 char。
 * 原实现直接把 (unsigned int *)(chars + i/2) 交给 sscanf，每个字节写 4 个字节，
 * 最后一个字节会越界写 3 个字节。正确做法是先读进局部变量再截断。
 */
char *app_common_hexstr2Chars(char *hexstr, int *chars_len)
{
    if (hexstr == NULL || chars_len == NULL)
    {
        return NULL;
    }
    // 得到hexstr的长度
    int hexstr_len = strlen(hexstr);
    // 指定chars_len的值
    *chars_len = hexstr_len/2;

    // 申请chars空间
    char *chars = (char *)malloc(*chars_len);
    if (chars == NULL)
    {
        return NULL;
    }
    // 遍历hexstr中的每2个16进制字符（以2个为单位），得到1个对应的字符，保存到chars中
    for(int i =0;i<hexstr_len;i+=2)
    {
        unsigned int byte = 0;
        sscanf(hexstr + i, "%02X", &byte);
        chars[i/2] = (char)byte;
    }
    // 返回chars
    return chars;
}