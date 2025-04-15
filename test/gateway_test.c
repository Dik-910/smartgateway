#include "app_runner.h"
#include "daemon_runner.h"
#include "log/log.h"
#include <string.h>
#include "ota_version.h"

int main(int argc, char const *argv[])
{
    // 检查参数
    if (argc == 1)
    {
        log_error("请携带参数运行");
        return -1;
    }

    if (strcmp(argv[1], "app") == 0)
    {
        app_runner_run();   // 运行app模块
    }
    else if (strcmp(argv[1], "ota") == 0)
    {
        ota_version_checkUpdateDaily();
    }
    else if (strcmp(argv[1], "daemon") == 0)
    {
        daemon_runner_run();
    }
    else
    {
        log_error("参数错误, 参数必须为：app|ota|daemon");
        return -1;
    }

    return 0;
}

// ./gateway_test daemon
// ./gateway_test app => 运行app模块
// ./gateway_test ota =》运行ota模块