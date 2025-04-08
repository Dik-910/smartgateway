#if !defined(__APP_MESSAGE_H)
#define __APP_MESSAGE_H

/**
 * @brief 字符数组消息转json消息
 * @param chars 字符数组消息
 * @param chars_len 字符数组长度
 * @return json消息
 */
char *app_message_chars2Json(char *chars, int chars_len);


/**
 * @brief json消息转字符数组消息
 * @param json json消息
 * @param chars_buf 字符数组消息的容器
 * @param buf_size 容器大小
 * @return 实际写入的字符数组长度，-1表示失败
 */
int app_message_json2Chars(char *json, char *chars_buf, int buf_size);

#endif // __APP_MESSAGE_H
