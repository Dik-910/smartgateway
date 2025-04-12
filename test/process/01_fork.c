#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    printf("主(父)线程(%d)开始执行\n", getpid());
    int val = 100;
    // 创建一个子进程
    /**
     * pid=-1: 创建子进程失败
     * pid=0: 子进程
     * pid>0(子进程id):父进程
     * 
     fork的过程：
     1. 创建子进程， 拷贝父进程的内存作为它的内存空间（两个进程当前内存数据一样）
     2. 如果选择执行父进程，返回子进程id
     3. 如果选择执行子进程，返回0
     */
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("创建子进程失败\n");
        return -1;
    }
    else if (pid == 0) // 子进程
    {
        val++;
        printf("子进程(%d)开始执行，父进程id:%d, val=%d, val地址=%p\n"
            , getpid(), getppid(), val, &val);
    }
    else  // 父进程
    {
        val--;
        printf("父进程(%d)又继续执行，子进程id:%d, val=%d, val地址=%p\n"
            , getpid(), pid, val, &val);
    }

    printf("进程(%d)执行即将结束\n", getpid());
    
    return 0;
}
