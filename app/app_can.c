#define _GNU_SOURCE
#include "app_can.h"
#include "log/log.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>

/**
 * CAN 帧转字符数组消息
 *
 * struct can_frame 的 can_id 高 3 位是标志位，不是 ID 本体：
 *   bit31 CAN_EFF_FLAG  1=29位扩展帧
 *   bit30 CAN_RTR_FLAG  1=远程帧（请求别人发数据，本身没有数据场）
 *   bit29 CAN_ERR_FLAG  1=错误帧（Bus Off 之类的总线故障通知）
 * 所以取 ID 之前必须先 & 掉标志位，否则拿到的 ID 是错的。
 */
int app_can_frame2Chars(const struct can_frame *frame, char *out, int out_size)
{
    if (frame == NULL || out == NULL)
    {
        log_error("CAN帧转消息：入参为空");
        return -1;
    }

    // 错误帧不是数据帧，交不出 data/id，由调用方单独处理
    if (frame->can_id & CAN_ERR_FLAG)
    {
        log_error("CAN帧转消息：这是错误帧，不能当数据处理");
        return -1;
    }

    // 经典 CAN 一帧最多 8 字节数据
    if (frame->can_dlc > CAN_MAX_DATA_LEN)
    {
        log_error("CAN帧转消息：数据长度非法 %d", frame->can_dlc);
        return -1;
    }

    // 扩展帧还是标准帧，以及去掉标志位后的 ID 本体
    int is_ext = (frame->can_id & CAN_EFF_FLAG) != 0;
    int id_len = is_ext ? CAN_EXT_ID_LEN : CAN_STD_ID_LEN;
    unsigned int id = (unsigned int)frame->can_id & (is_ext ? CAN_EFF_MASK : CAN_SFF_MASK);

    int need = 3 + id_len + frame->can_dlc;
    if (out_size < need)
    {
        log_error("CAN帧转消息：容器不足（需 %d，实 %d）", need, out_size);
        return -1;
    }

    // 3 字节消息头
    out[0] = CONN_TYPE_CAN;
    out[1] = (char)id_len;
    out[2] = (char)frame->can_dlc;

    // ID 按大端写入：高位在前，标准帧 2 字节，扩展帧 4 字节
    for (int i = 0; i < id_len; i++)
    {
        out[3 + i] = (char)((id >> (8 * (id_len - 1 - i))) & 0xFF);
    }

    // 数据场原样拷贝（字节序由上位机按 DBC 解释，网关不碰）
    if (frame->can_dlc > 0)
    {
        memcpy(out + 3 + id_len, frame->data, frame->can_dlc);
    }

    return need;
}

/**
 * 字符数组消息转 CAN 帧
 *
 * 这是下行方向：MQTT 收到上位机指令 -> 字符数组消息 -> CAN 帧 -> 写 can0
 */
int app_can_chars2Frame(const char *chars, int chars_len, struct can_frame *frame)
{
    if (chars == NULL || frame == NULL || chars_len < 3)
    {
        log_error("消息转CAN帧：入参非法");
        return -1;
    }

    // 长度字段按无符号解释，否则 0x80 以上会变成负数
    unsigned char conn_type = (unsigned char)chars[0];
    int id_len = (int)(unsigned char)chars[1];
    int data_len = (int)(unsigned char)chars[2];

    if (conn_type != CONN_TYPE_CAN)
    {
        log_error("消息转CAN帧：conn_type=%d 不是CAN消息", conn_type);
        return -1;
    }
    if (id_len != CAN_STD_ID_LEN && id_len != CAN_EXT_ID_LEN)
    {
        log_error("消息转CAN帧：id_len 非法 %d", id_len);
        return -1;
    }
    if (data_len > CAN_MAX_DATA_LEN)
    {
        log_error("消息转CAN帧：数据长度非法 %d", data_len);
        return -1;
    }
    if (chars_len != 3 + id_len + data_len)
    {
        log_error("消息转CAN帧：长度不一致（%d != %d）", chars_len, 3 + id_len + data_len);
        return -1;
    }

    // 按大端拼出 ID
    unsigned int id = 0;
    for (int i = 0; i < id_len; i++)
    {
        id = (id << 8) | (unsigned int)(unsigned char)chars[3 + i];
    }

    // 11 位标准帧只能到 0x7FF，29 位扩展帧只能到 0x1FFFFFFF
    unsigned int id_max = (id_len == CAN_EXT_ID_LEN) ? CAN_EFF_MASK : CAN_SFF_MASK;
    if (id > id_max)
    {
        log_error("消息转CAN帧：ID 0x%X 超出范围", id);
        return -1;
    }

    // 先清空整个结构体，避免 __pad/__res0/__res1 带脏数据上总线
    memset(frame, 0, sizeof(struct can_frame));
    frame->can_id = (canid_t)id | (id_len == CAN_EXT_ID_LEN ? CAN_EFF_FLAG : 0);
    frame->can_dlc = (unsigned char)data_len;
    if (data_len > 0)
    {
        memcpy(frame->data, chars + 3 + id_len, data_len);
    }

    return 0;
}

/**
 * CAN 读线程
 *
 * 和串口读线程的区别：串口 read 回来的是一段字节流，可能半帧也可能三帧半，
 * 所以 app_bt.c 里必须自己找 f1dd 头、读长度、拼包；CAN 的 read 每次
 * 必然返回恰好一个完整的 struct can_frame，边界由内核保证，不需要拼包。
 */
static void *can_read_thread_fun(void *arg)
{
    CanDevice *can = (CanDevice *)arg;
    struct can_frame frame;
    char chars[CAN_CHARS_MAX_LEN];

    while (can->is_running)
    {
        memset(&frame, 0, sizeof(frame));
        int n = read(can->fd, &frame, sizeof(frame));
        if (n < 0)
        {
            // 被信号打断不是错误，重试
            if (errno == EINTR)
            {
                continue;
            }
            log_error("读取CAN帧失败：%s", strerror(errno));
            break;
        }
        // 一次 read 要么是一整帧，要么什么都没有，长度不对说明有严重问题
        if (n != (int)sizeof(struct can_frame))
        {
            log_error("读取CAN帧长度异常：%d（期望 %d）", n, (int)sizeof(struct can_frame));
            continue;
        }

        // 错误帧：Bus Off / 错误被动 等总线故障都由它通知，不是业务数据
        if (frame.can_id & CAN_ERR_FLAG)
        {
            log_error("收到CAN错误帧：0x%08X", (unsigned int)frame.can_id);
            continue;
        }

        // 远程帧只有 ID 没有数据场，网关不做 RTR 转发
        if (frame.can_id & CAN_RTR_FLAG)
        {
            log_debug("忽略CAN远程帧");
            continue;
        }

        int len = app_can_frame2Chars(&frame, chars, sizeof(chars));
        if (len <= 0)
        {
            continue;
        }
        if (can->on_frame != NULL)
        {
            can->on_frame(chars, len);
        }
    }

    log_debug("CAN读线程退出");
    return NULL;
}

/**
 * 初始化 CAN 设备
 *
 * 四步固定流程（任何 SocketCAN 程序都一样）：
 *   1. socket(PF_CAN, SOCK_RAW, CAN_RAW)
 *   2. ioctl(SIOCGIFINDEX) 把接口名换成内核的接口索引
 *   3. bind 到该接口
 *   4. 按需 setsockopt（滤波器、错误帧掩码等）
 */
int app_can_init(CanDevice *can, const char *ifname, int (*on_frame)(char *chars, int chars_len))
{
    if (can == NULL || ifname == NULL)
    {
        log_error("CAN初始化：入参为空");
        return -1;
    }

    memset(can, 0, sizeof(CanDevice));
    can->fd = -1;
    can->on_frame = on_frame;
    strncpy(can->ifname, ifname, CAN_IFNAME_LEN - 1);

    // 1. 创建原始 CAN socket
    int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd < 0)
    {
        log_error("创建CAN socket失败：%s", strerror(errno));
        return -1;
    }

    // 2. 接口名换接口索引
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
    {
        log_error("获取CAN接口索引失败(%s)：%s", ifname, strerror(errno));
        close(fd);
        return -1;
    }

    // 3. 绑定到该接口
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        log_error("绑定CAN接口失败(%s)：%s", ifname, strerror(errno));
        close(fd);
        return -1;
    }

    // 4. 订阅错误帧。默认 RAW socket 是不收错误帧的，
    //    不订阅就永远不知道 Bus Off 发生过。
    //    失败不致命：个别驱动不支持，此时只是收不到总线故障事件。
    can_err_mask_t err_mask = CAN_ERR_MASK;
    if (setsockopt(fd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask)) < 0)
    {
        log_error("订阅CAN错误帧失败（不影响数据收发）：%s", strerror(errno));
    }

    can->fd = fd;
    log_debug("CAN接口 %s 初始化成功", ifname);

    return 0;
}

/**
 * 启动 CAN 读线程
 */
int app_can_start(CanDevice *can)
{
    if (can == NULL || can->fd < 0)
    {
        log_error("启动CAN失败：设备未初始化");
        return -1;
    }
    if (can->is_running)
    {
        log_debug("CAN读线程已经在运行");
        return 0;
    }

    can->is_running = 1;
    if (pthread_create(&can->read_thread, NULL, can_read_thread_fun, can) != 0)
    {
        log_error("创建CAN读线程失败");
        can->is_running = 0;
        return -1;
    }

    log_debug("CAN读线程已启动");
    return 0;
}

/**
 * 发送一帧 CAN 消息
 */
int app_can_send(CanDevice *can, const char *chars, int chars_len)
{
    if (can == NULL || can->fd < 0)
    {
        log_error("CAN发送失败：设备未初始化");
        return -1;
    }

    struct can_frame frame;
    if (app_can_chars2Frame(chars, chars_len, &frame) != 0)
    {
        return -1;
    }

    // 一次 write 就是整帧入队，内核不会把半帧放到总线上；
    // 但有硬件邮箱（通常 3 个）满了会阻塞或返回 EAGAIN，这里用阻塞写
    int n = write(can->fd, &frame, sizeof(frame));
    if (n != (int)sizeof(struct can_frame))
    {
        log_error("CAN发送失败：%s", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * 关闭 CAN 设备
 */
void app_can_close(CanDevice *can)
{
    if (can == NULL)
    {
        return;
    }

    can->is_running = 0;
    if (can->read_thread != 0)
    {
        // read 是取消点，pthread_cancel 能让阻塞在这里的线程退出
        pthread_cancel(can->read_thread);
        pthread_join(can->read_thread, NULL);
        can->read_thread = 0;
    }

    if (can->fd >= 0)
    {
        close(can->fd);
        can->fd = -1;
    }

    log_debug("CAN接口已关闭");
}
