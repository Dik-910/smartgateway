// 需求：利用系统调用实现love.txt的一个备份，并在控制台输出文件内容

#include <stdio.h> // perror()
#include <fcntl.h> // open()
#include <stdlib.h> // exit()
#include <unistd.h> // read()/write()/close()

int main(int argc, char const *argv[])
{
    // 1. 打开读/写文件流
    int read_fd = open("love.txt", O_RDONLY);
    if (read_fd == -1)
    {
        printf("打开读文件失败 \n");
        return -1;
    }
    int write_fd = open("love2.txt", O_CREAT|O_WRONLY|O_APPEND, 0644);
    if (write_fd == -1)
    {
        printf("打开写文件失败 \n");
        close(read_fd);
        return -1;
    }

    // 2. 不断边读边写， 并输出到控制台
    char buffer[1024];
    ssize_t read_len;
    while((read_len = read(read_fd, buffer, sizeof(buffer))) > 0)
    {
        write(write_fd, buffer, read_len);
        write(STDOUT_FILENO, buffer, read_len);
    }

    // 3. 关闭文件流
    close(read_fd);
    close(write_fd);

    return 0;
}
