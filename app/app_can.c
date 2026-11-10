#include "app_can.h"
#include "log/log.h"
#include <string.h>

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
