#include "cJSON/cJSON.h"
#include "log/log.h"
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // 1. 生成json字符串
    cJSON *root_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(root_obj, "name", "tom");
    cJSON_AddNumberToObject(root_obj, "age", 18);
    char *json = cJSON_PrintUnformatted(root_obj);
    log_debug("生成json字符串: %s", json);

    // 2. 解析json字符串
    cJSON * root_obj2 = cJSON_Parse(json);
    if (root_obj2 == NULL) {
        log_error("解析json字符串失败");
        return -1;
    }
    char *name = cJSON_GetObjectItem(root_obj2, "name")->valuestring;
    int age = cJSON_GetObjectItem(root_obj2, "age")->valueint;
    log_debug("解析json字符串: name=%s, age=%d", name, age);

    // 3. 释放内存
    cJSON_Delete(root_obj);
    cJSON_Delete(root_obj2);
    free(json);

    return 0;
}
