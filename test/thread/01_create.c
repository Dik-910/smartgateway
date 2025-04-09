#define _GNU_SOURCE  // 声明可以使用扩展语法
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// first线程函数
void *first_thread_fun(void *arg)
{
    printf("first线程(%d)开始执行 \n", gettid());

    return NULL;
}

// second线程函数
void *second_thread_fun(void *arg)
{
    printf("second线程(%d)开始执行 \n", gettid());

    return NULL;
}

int main(int argc, char const *argv[])
{
    printf("主线程(%d)开始执行 \n", gettid());
    // 创建2个分线程
    pthread_t pt1, pt2;  // 声明线程标识变量
    pthread_create(&pt1, NULL, first_thread_fun, NULL);
    pthread_create(&pt2, NULL, second_thread_fun, NULL);
    printf("主线程(%d)继续执行\n" ,gettid());
    // 等待2个分线程执行完毕
    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    printf("主线程(%d)即将结束执行, pt1=%ld, pt2=%ld \n", gettid(), pt1, pt2);

    return 0;
}
