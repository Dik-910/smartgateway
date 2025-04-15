#include "log/log.h"
#include "ota_version.h"
#include <unistd.h>
#include "ota_http.h"
#include "cJSON/cJSON.h"
#include <openssl/sha.h>
#include <sys/reboot.h>
#include <string.h>
#include <stdlib.h>

/**
 * 获取文件的SHA1哈希值（40位16进制字符串）
 * 相同文件返回相同的哈希值，可用于判断文件是否相同
 * linux命令生成：sha1sum 文件名
 * 此函数可以利用AI工具帮我们生成
 */
static char *get_file_sha(char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        perror("Failed to open file");
        return NULL;
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha1;
    SHA1_Init(&sha1);

    const int bufSize = 32768;
    unsigned char *buffer = (unsigned char *)malloc(bufSize);
    if (!buffer)
    {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    int bytesRead;
    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0)
    {
        SHA1_Update(&sha1, buffer, bytesRead);
    }

    SHA1_Final(hash, &sha1);
    fclose(file);
    free(buffer);

    char *outputBuffer = (char *)malloc(SHA_DIGEST_LENGTH * 2 + 1);
    if (!outputBuffer)
    {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    return outputBuffer;
}

/**
 * 版本检查更新
 * 1. 获取远程版本信息的json
 * 2. 解析json, 得到版本号+固件hash值
 * 3. 比较本地版本号和远程版本号， 如果本地不小于远程，则不更新，直接结束
 * 4. 如果本地版本号小于远程版本号，则开始请求下载固件
 * 5. 下载成功，则开始校验固件hash值（下载的和json解析出的）
 * 6. 如果校验失败， 删除下载的文件，则失败结束
 * 7. 如果校验成功，重启系统，运行最新的固件
 */
int ota_version_checkUpdate()
{
    // 获取远程版本信息的json
    char *json = ota_http_getJson(OTA_URL_FILEINFO);
    // 解析json, 得到版本号+固件hash值
    cJSON *root = cJSON_Parse(json);
    int major = cJSON_GetObjectItem(root, "major")->valueint;
    int minor = cJSON_GetObjectItem(root, "minor")->valueint;
    int patch = cJSON_GetObjectItem(root, "patch")->valueint;
    log_debug("线上版本号：%d.%d.%d", major, minor, patch);
    log_debug("当前版本号：%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    // 比较本地版本号和远程版本号， 如果本地不小于远程，则不更新，直接结束
    if (VERSION_MAJOR > major 
        || (major == VERSION_MAJOR && VERSION_MINOR > minor) 
        || (major == VERSION_MAJOR && minor == VERSION_MINOR && VERSION_PATCH >= patch))
    {
        log_debug("当前已经是最新版本，无需更新");
        return 0;
    }

    // 如果本地版本号小于远程版本号，则开始请求下载固件
    int result = ota_http_download(OTA_URL_DOWNLOAD, OTA_LOCAL_FILE_PATH);
    if (result != 0) // 下载失败，则失败结束
    {
        log_error("下载固件失败");
        return -1;
    }
    // 下载成功，则开始校验固件hash值（下载的和json解析出的）
    char *remote_sha = cJSON_GetObjectItem(root, "sha1")->valuestring;
    char *local_sha = get_file_sha(OTA_LOCAL_FILE_PATH);
    // 如果校验失败， 删除下载的文件，则失败结束
    if (strcmp(remote_sha, local_sha) != 0)
    {
        log_error("固件校验失败");
        unlink(OTA_LOCAL_FILE_PATH);
        free(remote_sha);
        free(local_sha);
        cJSON_Delete(root);
        return -1;
    }

    // 如果校验成功，重启系统，运行最新的固件
    log_debug("固件校验成功，开始重启系统");
    reboot(RB_AUTOBOOT); // 重启系统 =》 需要当前是root
    return 0;
}

/**
 * 每隔一天检查一次版本更新
 */
int ota_version_checkUpdateDaily()
{
    while (1)
    {
        ota_version_checkUpdate();
        log_debug("等待一天后再次检查版本更新");
        sleep(24 * 60 * 60); // 24小时
    }
}

/**
 * 输出版本号
 */
void ota_version_printVersion()
{
    log_debug("-当前版本号：%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}