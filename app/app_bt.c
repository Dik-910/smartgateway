#include "app_bt.h"
#include <string.h>
#include "log/log.h"

/**
 * 蓝牙模块初始化
 * 1. 给设备指定preWrite和postRead两个函数
 * 2. 蓝牙连接初始化配置
 */
int app_bt_init(Device *device)
{
    device->post_read = app_bt_postRead;
    device->pre_write = app_bt_preWrite;
}

/**
 * 蓝牙数据的写前处理函数
字符数组消息: 
    例子：1 2 3 XX abc
    格式：conn_type id_len msg_len id msg
蓝牙发送数据格式例子：
    例子：41 54 2b 4d 45 53 48 00 ff ff 61 62 63 0d 0a
    41 54 2b 4d 45 53 48 00： AT+MESH（固定头部）
    ff ff: 对端的MADDR（如果是FFFF代表群发）
    61 62 63: 要发送的数据（不超过12字节）
    0d 0a：\r\n（固定结尾）
    AT+MESH XX abc \r\n
 */
int app_bt_preWrite(char *data, int data_len)
{
    // 检查data的长度是否合法
    if (data_len < 6)
    {
        log_error("要处理的消息长度不能小于6");
        return -1;
    }

    // 计算蓝牙数据的长度
    int blue_len = 8 + 2 + data[2] + 2;
    // 创建蓝牙数据数组容器
    char blue_data[blue_len];
    // 根据data中的数据组装蓝牙数据
    // data: 1 2 3 XX abc
    // blue_data: AT+MESH XX abc \r\n
    // 拷贝AT+MESH
    memcpy(blue_data, "AT+MESH", 8);
    // 拷贝id
    memcpy(blue_data+8, data+3, 2);
    // 拷贝msg
    memcpy(blue_data+10, data+5, data[2]);
    // 拷贝\r\n
    memcpy(blue_data+10+data[2], "\r\n", 2);

    // 清空data中的数据
    memset(data, 0, data_len);

    // 将蓝牙数据拷贝到data中
    memcpy(data, blue_data, blue_len);

    // 返回蓝牙数据长度
    return blue_len;
}

 /**
  * 蓝牙数据的读后处理函数
  */
int app_bt_postRead(char *data, int data_len)
{
    
}