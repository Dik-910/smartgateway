#include "app_device.h"
#include "unistd.h"
#include "stdlib.h"
#include "app_bt.h"
#include "signal.h"
#include "app_runner.h"

int is_running = 1;

void runner_exit(int sig)
{
    is_running = 0;
}

int app_runner_run()
{
    // 注册信号处理函数来实现结束前释放资源的操作
    signal(SIGINT, runner_exit);
    signal(SIGTERM, runner_exit);

    // 初始化设备
    Device *device = app_device_init(DEVICE_FILE);
    // 初始化蓝牙
    app_bt_init(device);
    // 启动设备
    app_device_start();
    // 不断运行
    while (is_running == 1)
    {
        sleep(1);
    }

    // 释放资源
    app_device_close();

    return 0;
}