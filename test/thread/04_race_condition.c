#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 20000

void *add_thread(void *argv)
{
    // 将接收的v参数转换为int指针
    int *p = argv;

    // 对指向的内存int数据加1,也就是给num加1
    *p = *p + 1;

    return NULL;
}

int main()
{
    pthread_t threads[THREAD_COUNT];

    int num = 0; // 被n个线程进行不断累加的变量

    // 启动20000个线程对num进行累加
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&threads[i], NULL, add_thread, &num);
    }

    // 等待所有线程执行的结果
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // 打印累加结果 => 会比20000要小一些
    printf("累加结果：%d \n", num);

    return 0;
}