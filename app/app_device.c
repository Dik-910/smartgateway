#define _GNU_SOURCE
#include "app_device.h"
#include "app_message.h"
#include "app_common.h"
#include "app_mqtt.h"
#include "app_pool.h"
#include "app_can.h"
#include "log/log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

static Device *device = NULL; // 整个应用中只有一个Device对象
static CanDevice can_device;  // 整个应用中只有一个CAN设备对象

// CAN 收到一帧后的回调，实现在本文件下方
static int can_frame_callback(char *chars, int chars_len);
// 上行发送任务的线程函数，实现在本文件下方
static int send_msg_fun(void *arg);

/**
 * 初始化Device对象
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

    // 初始化CAN链路
    // 注意：CAN 起不来不能拖垮整个网关。车载网关的可用性要求是
    // “任意一路链路故障，其余链路必须继续工作”，所以这里只告警不返回错误。
    if (app_can_init(&can_device, CAN_DEFAULT_IFNAME, can_frame_callback) != 0)
    {
        log_error("CAN链路初始化失败(%s)，CAN不可用，蓝牙链路继续工作", CAN_DEFAULT_IFNAME);
    }

    // 返回Device指针
    return device;
}

/**
 * CAN收到一帧数据后的回调（在CAN读线程中执行）
 *
 * 和蓝牙的 post_read 不同：蓝牙数据要先拼包（app_bt_postRead），
 * CAN 的帧边界由内核保证，读出来就已经是完整的字符数组消息了，
 * 这里直接写上行缓冲区、把发送任务交给线程池。
 */
static int can_frame_callback(char *chars, int chars_len)
{
    if (device == NULL || chars == NULL || chars_len <= 0)
    {
        return -1;
    }

    // 和蓝牙共用同一个上行缓冲区，靠消息里的 conn_type 区分来源
    if (app_buffer_write(device->up_buffer, chars, chars_len) != 0)
    {
        log_error("CAN消息写入上行缓冲区失败");
        return -1;
    }
    // 后面的 JSON 转换与 MQTT 发送交给线程池，和蓝牙走同一条路
    return app_pool_addTask(send_msg_fun, NULL);
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
        // log_debug("------: %ld", data_len);
        // 将蓝牙数据转换为字符数组Message
        if (data_len > 0 && device->post_read)
        {
            data_len = device->post_read(data_buf, data_len);
        }

        if (data_len > 0)
        {
            // 将Message写入上行缓冲区
            app_buffer_write(device->up_buffer, data_buf, data_len);

            // 将发送消息的任务交给线程池模块完成（添加一个发送消息到远程的任务: send_msg_fun）
            app_pool_addTask(send_msg_fun, NULL);
        }
    }
}

// 读取下行缓冲区数据处理后写入蓝牙串口文件传递给下游设备的任务函数（线程池中某个线程函数调用）
// 两次写入的时间差要>=200
static int write_task_fun(void *arg)
{
    Device *device = (Device *)arg;
    // 从下行缓冲区中读取一个字符数组message数据
    char data_buf[128];
    int data_len = app_buffer_read(device->down_buffer, data_buf, sizeof(data_buf));
    // 将字符数组message转换为蓝牙数据
    if (device->pre_write)
    {
        data_len = device->pre_write(data_buf, data_len);
    }

    // 在准备写入文件前，限制两次写入的时间差要>=200
    long distance = app_common_getCurrentTime() - device->last_write_time;
    if (distance < 200)
    {
        usleep((200-distance)*1000);
    }

    // 将蓝牙数据写入蓝牙串口文件
    ssize_t len  = write(device->fd, data_buf, data_len);
    if (len != data_len)
    {
        log_error("写入蓝牙串口文件失败");
        return -1;
    }
    log_debug("写入蓝牙串口文件成功: %.*s", data_len, data_buf);
    // 保存时间
    device->last_write_time = app_common_getCurrentTime();

    return 0;
}

// 处理接收的远程消息的回调函数
static int recv_msg_callback(char *json)
{
    // json消息转换为字符数组Message
    char data_buf[128];
    int data_len = app_message_json2Chars(json, data_buf, sizeof(data_buf));
    if (data_len <= 0)
    {
        log_error("下行消息解析失败");
        return -1;
    }

    // 按 conn_type 分流：CAN 消息直接写 CAN socket，其余走蓝牙串口
    //
    // 这里不能复用 write_task_fun：它固定从 down_buffer 取数据、固定写
    // device->fd（蓝牙串口）。如果让 CAN 也走它，两个任务会从同一个
    // down_buffer 里取到发给对方的报文 —— 这就是串台。
    // 因此 CAN 下行单独一条同步路径：CAN 上行是高频数据，下行是上位机
    // 指令，频率在秒级，一次 write 就是整帧入队，同步发送的代价可以忽略。
    if ((unsigned char)data_buf[0] == CONN_TYPE_CAN)
    {
        if (app_can_send(&can_device, data_buf, data_len) != 0)
        {
            log_error("CAN下行发送失败");
            return -1;
        }
        return 0;
    }

    // 将Message写入下行缓冲区
    if (app_buffer_write(device->down_buffer, data_buf, data_len) != 0)
    {
        log_error("下行消息写入缓冲区失败");
        return -1;
    }
    // 将后面写入蓝牙串口文件的任务交给线程池模块完成（添加一个将下行缓冲区数据写入蓝牙串口文件的任务）
    return app_pool_addTask(write_task_fun, device);
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

    // 启动 CAN 上行流程：启动 CAN 独立读线程（方案A，与串口读线程互不阻塞）
    // CAN 初始化失败时 fd 为 -1，这里会打日志跳过，不影响蓝牙链路
    if (app_can_start(&can_device) != 0)
    {
        log_error("CAN读线程启动失败，CAN链路不可用");
    }

    // 启动下行流程: 给MQTT模块注册一个接收远程消息的回调函数
    app_mqtt_registerRecvCallback(recv_msg_callback);

    return 0;
}

/**
 * 关闭
 */
void app_device_close()
{
    // 关闭CAN链路（先停读线程，再关socket）
    app_can_close(&can_device);
    // 关闭文件
    close(device->fd);
    // 释放buffer
    app_buffer_free(device->up_buffer);
    app_buffer_free(device->down_buffer);
    // 取消读线程
    pthread_cancel(device->read_thread);
    pthread_join(device->read_thread, NULL);
    // 释放Device内存
    free(device);

    // 关闭线程池
    app_pool_destroy();
    // 关闭MQTT模块
    app_mqtt_close();
}