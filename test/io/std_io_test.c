#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/**
标准输入输出流 （File *）
    FILE *stdin;   标准输入
    FILE *stdout;  标准输出
    FILE *stderr;  标准错误输出
标准输出输出流的文件描述符
    STDIN_FILENO    0  标准输入
    STDOUT_FILENO   1  标准输出
    STDERR_FILENO   2  标准错误输出
 */

int main(int argc, char const *argv[])
{
    // 关闭标准输出和标准错误输出
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    // 打开文件1（标准输出）
    open("out.log", O_CREAT|O_RDWR|O_APPEND, 0644);
    // 打开文件2（标准错误输出）
    open("error.log", O_CREAT|O_RDWR|O_APPEND, 0644);

    // 使用stdin读取控制台输入
    char input_name[20];
    fgets(input_name, 20, stdin);
    printf("stdin读取输入：%s\n", input_name);
    // 使用stdout输出到控制台
    fputs("stdout输出到控制台 \n", stdout);
    fflush(stdout); // 刷新缓冲区：将缓冲区内准备写的数据立即写入目标
    // 使用stderr输出到控制台
    fputs("stderr输出到控制台 \n", stderr);

    // 使用STDIN_FILENO读取控制台输入
    char input_name2[20];
    int len = read(STDIN_FILENO, input_name2, 20);
    printf("STDIN_FILENO读取输入:%s,len=%d\n", input_name2, len);

    // 使用STDOUT_FILENO输出到控制台
    write(STDOUT_FILENO, "STDOUT_FILENO输出到控制台 \n", strlen("STDOUT_FILENO输出到控制台 \n"));
    // 使用STDERR_FILENO输出到控制台
    write(STDERR_FILENO, "STDERR_FILENO输出到控制台 \n", strlen("STDERR_FILENO输出到控制台 \n"));
    
    return 0;
}
