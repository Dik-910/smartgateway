#include "app_runner.h"
#include "log/log.h"
#include <string.h>

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
    else
    {
        log_error("参数错误");
        return -1;
    }
    return 0;
}


// ./gateway app