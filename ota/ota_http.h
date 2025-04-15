#if !defined(__OTA_HTTP_H)
#define __OTA_HTTP_H
#define OTA_URL_FILEINFO "http://192.168.11.91:8000/fileinfo.json" // 版本信息地址
#define OTA_URL_DOWNLOAD "http://192.168.11.91:8000/download/gateway" // 下载地址
// #define OTA_LOCAL_FILE_PATH "/home/zxf/gateway.update"
#define OTA_LOCAL_FILE_PATH "/root/gateway.update"


/**
 * 请求指定url获取json数据
 */
char *ota_http_getJson(char *url);

/**
 * 请求指定url下载文件到路径位置
 */
int ota_http_download(char *url, char *filename);


#endif // __OTA_HTTP_H
