#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
    printf("主(父)线程(%d)开始执行\n", getpid());

    pid_t pid = fork();
    if (pid == 0) // 子进程
    {
        printf("子进程(%d)开始执行 \n", getpid());

        // 通过execve()执行other
        char *path = "/home/zxf/桌面/gateway/test/process/other";
        char *argv[] = {path, "abc", NULL}; // argv数组的最后一个元素必须是NULL
        char *envp[] = {NULL}; // 环境变量数组，最后一个元素必须是NULL => 使用默认环境变量
        execve(path, argv, envp);
        printf("execve()执行失败\n");
    }
    else  // 父进程
    {
        printf("父进程(%d)又继续执行\n", getpid());
    }

    // return 0;
    // _exit(0);
    exit(EXIT_SUCCESS);
}
