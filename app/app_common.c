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
 */
char *app_common_chars2Hexstr(char *chars, int chars_len)
{
    // 申请16进制字符串空间
    char *hexstr = (char *)malloc(chars_len*2 + 1);
    // 遍历chars中的每个字符，将字符转换为二位16进制字符保存到hexstr中
    for (int i = 0; i < chars_len; i++)
    {
        sprintf(hexstr + i*2, "%02X", chars[i]);
    }
    // 向hexstr中添加结束符
    hexstr[chars_len*2] = '\0';

    // 返回hexstr
    return hexstr;
}

/**
 * "61626364" => [abcd]
 */
char *app_common_hexstr2Chars(char *hexstr, int *chars_len)
{
    // 得到hexstr的长度
    int hexstr_len = strlen(hexstr);
    // 指定chars_len的值
    *chars_len = hexstr_len/2;

    // 申请chars空间
    char *chars = (char *)malloc(*chars_len);
    // 遍历hexstr中的每2个16进制字符（以2个为单位），得到1个对应的字符，保存到chars中
    for(int i =0;i<hexstr_len;i+=2)
    {
        sscanf(hexstr + i, "%02X", (unsigned int *)(chars + i/2));
    }
    // 返回chars
    return chars;
}