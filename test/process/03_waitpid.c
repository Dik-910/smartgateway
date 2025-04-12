#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    printf("主(父)线程(%d)开始执行\n", getpid());

    pid_t pid = fork();
    if (pid == 0) // 子进程
    {
        printf("子进程(%d)开始执行 \n", getpid());

        // 通过execve()执行命令
        char *path = "/bin/ping";  // ping www.baidu.com -c 5
        char *argv[] = {path, "www.baidu.com", "-c", "5", NULL}; // argv数组的最后一个元素必须是NULL
        execve(path, argv, NULL);
        printf("execve()执行失败\n");
    }
    else  // 父进程
    {
        printf("父进程(%d)又继续执行\n", getpid());

        // 等待子进程执行结束
        int result;
        waitpid(pid, &result, 0);
        printf("子进程执行结束，回到主进程执行，result=%d\n", result);
    }

    printf("进程(%d)结束执行\n", getpid());

    // return 0;
    // _exit(0);
    exit(EXIT_SUCCESS);
}
