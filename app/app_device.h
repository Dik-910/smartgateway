#if !defined(__APP_DEVICE_H)
#define __APP_DEVICE_H

#include "app_buffer.h"
#include <pthread.h>

// 设备的结构体
typedef struct
{
    char *filename; // 串口文件名
    int fd; // 串口文件描述符
    Buffer *up_buffer; // 上行缓冲区
    Buffer *down_buffer; // 下行缓冲区
    pthread_t read_thread; // 读取串口文件中数据的线程
    int is_running; // 读线程是否正在运行
    int (*post_read)(char *data, int len); // 读取到串口数据后的后置处理函数（蓝牙数据=》message）
    int (*pre_write)(char *data, int len); // 对向串口写入的数据的前置处理函数（message=》蓝牙数据）
} Device;


/**
 * 初始化
 */
Device *app_device_init(char *filename);

/**
 * 启动
 */
int app_device_start();

/**
 * 关闭
 */
void app_device_close();

#endif // __APP_DEVICE_H
