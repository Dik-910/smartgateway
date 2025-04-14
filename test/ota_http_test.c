#include "ota_http.h"
#include "log/log.h"

int main(int argc, char const *argv[])
{
    char *json = ota_http_getJson(OTA_URL_FILEINFO);
    log_debug("json: %s", json);

    int result = ota_http_download(OTA_URL_DOWNLOAD, OTA_LOCAL_FILE_PATH);
    if (result==-1)
    {
        log_error("下载失败");
        return -1;
    }
    log_debug("下载成功");
    return 0;
}
