#include "app_device.h"
#include "app_message.h"
#include "app_common.h"
#include "app_mqtt.h"
#include "app_pool.h"
#include "log/log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

static Device *device = NULL; // 整个应用中只有一个Device对象
/**
 * 初始化
 */
Device *app_device_init(char *filename)
{
    if (device)
    {
        return device;
    }
    // 申请Device内存
    device = (Device *)malloc(sizeof(Device));
    // 初始化Device中的各个成员
    device->filename = filename;
    device->fd = open(filename, O_RDWR);
    device->up_buffer = app_buffer_init(BUFFER_SIZE);
    device->down_buffer = app_buffer_init(BUFFER_SIZE);
    device->is_running = 0;
    device->post_read = NULL;
    device->pre_write = NULL;

    // 初始化线程池
    app_pool_init(5);
    // 初始化MQTT模块
    app_mqtt_init();

    // 返回Device指针
    return device;
}

/**
 * 发送消息的任务函数（线程池中某个线程函数调用）
 */
static int send_msg_fun(void *arg)
{
    // 从上行缓冲区中读取一个字符数组message数据
    char data_buf[128];
    int  data_len = app_buffer_read(device->up_buffer, data_buf, sizeof(data_buf));
    // 将字符数组message转换为json
    char *json = app_message_chars2Json(data_buf, data_len);
    // 将json数据发送给远程
    int result = app_mqtt_send(json);
    if (result == -1)
    {
        log_error("发送消息失败");
        return -1;
    }
    log_debug("发送消息成功: %s", json);

    return 0;
}

// 不断从串口文件中读取数据的线程函数
static void *read_thread_fun(void *arg)
{
    while(device->is_running)
    {
        // 从串口文件中读取数据   蓝牙数据
        char data_buf[128];
        ssize_t data_len = read(device->fd, data_buf, sizeof(data_buf));

        // 将蓝牙数据转换为字符数组Message
        if (data_len > 0 && device->post_read)
        {
            data_len = device->post_read(data_buf, data_len);

            // 将Message写入上行缓冲区
            app_buffer_write(device->up_buffer, data_buf, data_len);

            // 将发送消息的任务交给线程池模块完成（添加一个发送消息到远程的任务: send_msg_fun）
            app_pool_addTask(send_msg_fun, NULL);
        }
    }
}

/**
 * 启动
 */
int app_device_start()
{
    if (device->is_running)
    {
        log_debug("读线程已经启动运行中");
        return 0;
    }
    device->is_running = 1;
    // 启动上行流程： 启动读线程
    pthread_create(&device->read_thread, NULL, read_thread_fun, NULL);

    // 启动下行流程: 给MQTT模块注册一个接收远程消息的回调函数
}

/**
 * 关闭
 */
void app_device_close()
{
    // 关闭文件
    close(device->fd);
    // 释放buffer
    app_buffer_free(device->up_buffer);
    app_buffer_free(device->down_buffer);
    // 取消读线程
    pthread_cancel(device->read_thread);
    phtread_join(device->read_thread, NULL);
    // 释放Device内存
    free(device);

    // 关闭线程池
    app_pool_destroy();
    // 关闭MQTT模块
    app_mqtt_close();
}