#if !defined(__APP_COMMON_H)
#define __APP_COMMON_H

/**
 * @brief 获取当前时间戳   ms单位
 * @return long  当前时间戳
 */
long app_common_getCurrentTime();

/**
 * @brief 字符数组转十六进制字符串
 * @param chars 字符数组
 * @param chars_len 字符数组长度
 * @return char* 十六进制字符串
 */
char *app_common_chars2Hexstr(char *chars, int chars_len);

/**
 * @brief 十六进制字符串转字符数组
 * @param hexstr 十六进制字符串
 * @param chars_len 接收字符数组长度的指针
 * @return char* 字符数组
 */
char *app_common_hexstr2Chars(char *hexstr, int *chars_len);

#endif // __APP_COMMON_H
