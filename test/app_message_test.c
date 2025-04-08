#include "app_message.h"
#include "log/log.h"
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *json_message = "{\"conn_type\":1,\"id\":\"5858\",\"msg\":\"61626364\"}";

    // json消息转b字符数组消息
    char chars_buf[100];
    int len = app_message_json2Chars(json_message, chars_buf, 100);
    log_debug("字符数组消息： %.*s, 长度：%d", len, chars_buf, len);

    // 字符数组消息转json消息
    char *json_message2 = app_message_chars2Json(chars_buf, len);
    log_debug("json消息： %s", json_message2);

    free(json_message2);
    
    return 0;
}
