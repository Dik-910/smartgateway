#include "mqueue.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static char *mq_name = "/my_mq";  // /代表： /dev/mqueue目录
static mqd_t mq_fd;  // 队列描述符   // description描述符

// first线程函数
void *first_thread_fun(void *arg)
{
    sleep(1);

    printf("first线程开始执行, 发送消息 \n");
    // 发送消息
    mq_send(mq_fd, "hello", 6, 0);

    return NULL;
}

// second线程函数
void *second_thread_fun(void *arg)
{
    
    
    printf("second线程开始执行，接收消息 \n");

    // 接收消息
    char buf[100];
    ssize_t len = mq_receive(mq_fd, buf, 101, NULL);
    printf("接收到的消息是：%s, 长度是：%ld \n", buf, len);

    return NULL;
}

int main(int argc, char const *argv[])
{
    printf("主线程开始执行 \n");

    // 创建队列
    struct mq_attr attr;  // attribute属性
    attr.mq_maxmsg = 10;  // 最大消息数
    attr.mq_msgsize = 100;  // 每条消息的最大字节数

    // O_CREAT: 如果不存在创建 O_RDWR：可读可写  0644：是创建的文件的权限组合
    mq_fd = mq_open(mq_name, O_CREAT | O_RDWR, 0644, &attr);
    if (mq_fd == -1)
    {
        printf("创建队列失败\n");
        return -1;
    }


    pthread_t pt1, pt2;  // 声明线程标识变量
    pthread_create(&pt1, NULL, first_thread_fun, NULL);
    pthread_create(&pt2, NULL, second_thread_fun, NULL);
    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    printf("主线程即将结束执行\n");

    // 关闭并删除队列
    mq_close(mq_fd);
    mq_unlink(mq_name);

    return 0;
}
