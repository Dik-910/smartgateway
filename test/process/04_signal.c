#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
int is_running = 1;


void use_resource()
{
    printf("使用资源\n");
}

void release_resource()
{
    printf("释放资源\n");
}

void handle_callback(int signum)
{
    if (signum==SIGINT)
    {
        printf("收到SIGINT信号\n");
        release_resource(); // 释放资源
        exit(EXIT_SUCCESS);  // 手动退出程序
    }
    else if (signum==SIGTERM)
    {
        printf("收到SIGTERM信号\n");
        release_resource(); // 释放资源
        exit(EXIT_SUCCESS);
    }
}


int main(int argc, char const *argv[])
{
    // 注册信号处理函数
    signal(SIGINT, handle_callback);  //  CTRL+C
    signal(SIGTERM, handle_callback); // kill
    
    use_resource();

    while (is_running)
    {
        printf("正在运行\n");
        sleep(1);
    }
    

    release_resource();
    return 0;
}
