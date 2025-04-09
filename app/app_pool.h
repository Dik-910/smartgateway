#if !defined(__APP_POOL_H)
#define __APP_POOL_H

// 任务结构体
typedef struct
{
    int (*task_fun)(void *arg); // 任务函数
    void *arg; // 任务函数执行时需要的参数
} Task;


/**
 * 初始化线程池
 */
int app_pool_init(int size);

/**
 * 销毁线程池
 */
void app_pool_destroy();

/**
 * 添加任务
 */
int app_pool_addTask(int (*task_fun)(void *arg), void *arg);

#endif // __APP_POOL_H
