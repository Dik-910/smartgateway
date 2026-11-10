#if !defined(__APP_CAN_H)
#define __APP_CAN_H

#include <linux/can.h>
#include <pthread.h>

/**
 * 字符数组消息（conn_type + id_len + msg_len + id + msg）中，
 * conn_type 用来区分消息来自哪条链路：
 *   1 = 蓝牙 Mesh（app_bt.c）
 *   2 = CAN（app_can.c）
 */
#define CONN_TYPE_CAN 2

// 标准帧的 11 位 ID 占 2 字节，扩展帧的 29 位 ID 占 4 字节
#define CAN_STD_ID_LEN 2
#define CAN_EXT_ID_LEN 4

// 经典 CAN 单帧数据上限 8 字节（CAN FD 才是 64）
#define CAN_MAX_DATA_LEN 8

// 一条 CAN 字符数组消息的最大长度：3 字节头 + 4 字节 ID + 8 字节数据
#define CAN_CHARS_MAX_LEN (3 + CAN_EXT_ID_LEN + CAN_MAX_DATA_LEN)

// 网络接口名的最大长度，与内核 IFNAMSIZ 一致
#define CAN_IFNAME_LEN 16

// CAN 设备：一个 socket + 一个独立读线程
typedef struct
{
    char ifname[CAN_IFNAME_LEN];    // 接口名，比如 "can0"、"vcan0"
    int fd;                         // AF_CAN 的 socket 描述符
    pthread_t read_thread;          // 读线程标识
    volatile int is_running;        // 读线程是否正在运行
    // 收到一帧并转成字符数组消息后的回调（交给 app_device 写上行缓冲区）
    int (*on_frame)(char *chars, int chars_len);
} CanDevice;

/**
 * @brief CAN 帧转字符数组消息
 *
 * struct can_frame(16字节) => [conn_type=2][id_len][msg_len][id...][data...]
 * 标准帧 id_len=2，扩展帧 id_len=4，ID 一律按大端写入。
 *
 * @param frame CAN 帧（读到的帧或待发送的帧）
 * @param out 字符数组消息的容器
 * @param out_size 容器大小，至少 3 + id_len + dlc
 * @return 实际写入的字符数组长度，-1 表示失败
 */
int app_can_frame2Chars(const struct can_frame *frame, char *out, int out_size);

/**
 * @brief 字符数组消息转 CAN 帧
 *
 * [conn_type=2][id_len][msg_len][id...][data...] => struct can_frame
 * id_len=2 生成标准帧，id_len=4 生成带 CAN_EFF_FLAG 的扩展帧。
 *
 * @param chars 字符数组消息
 * @param chars_len 字符数组消息的长度
 * @param frame 输出的 CAN 帧
 * @return 0 成功，-1 失败
 */
int app_can_chars2Frame(const char *chars, int chars_len, struct can_frame *frame);

/**
 * @brief 初始化 CAN 设备：创建 socket、绑定接口、订阅错误帧
 *
 * @param can CAN 设备对象
 * @param ifname 接口名，比如 "can0"、"vcan0"
 * @param on_frame 收到一帧并转成字符数组消息后的回调，可为 NULL
 * @return 0 成功，-1 失败
 */
int app_can_init(CanDevice *can, const char *ifname, int (*on_frame)(char *chars, int chars_len));

/**
 * @brief 启动 CAN 读线程（独立于串口读线程，互不阻塞）
 *
 * @return 0 成功，-1 失败
 */
int app_can_start(CanDevice *can);

/**
 * @brief 发送一帧 CAN 消息
 *
 * @param chars 字符数组消息（conn_type 必须是 CONN_TYPE_CAN）
 * @param chars_len 字符数组消息长度
 * @return 0 成功，-1 失败
 */
int app_can_send(CanDevice *can, const char *chars, int chars_len);

/**
 * @brief 关闭 CAN 设备：停止读线程并关闭 socket
 */
void app_can_close(CanDevice *can);

#endif // __APP_CAN_H
