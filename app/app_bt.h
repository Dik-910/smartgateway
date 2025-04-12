#if !defined(__APP_BT_H)
#define __APP_BT_H

#include "app_device.h"

typedef enum
{
    BT_BR_9600 = '4',
    BT_BR_115200 = '8',
} BT_BaudRate;

/**
 * 蓝牙模块初始化
 * 1. 给设备指定preWrite和postRead两个函数
 * 2. 蓝牙连接初始化配置
 */
int app_bt_init(Device *device);

/**
 * 蓝牙数据的写前处理函数
 */
int app_bt_preWrite(char *data, int data_len);

 /**
  * 蓝牙数据的读后处理函数
  */
int app_bt_postRead(char *data, int data_len);

/**
 * 判断蓝牙是否可用
 */
int app_bt_status(Device *device);

/**
 * 修改名称
 */
int app_bt_rename(Device *device, char *name);

/**
 * 设置波特率
 */
int app_bt_setBaudRate(Device *device, BT_BaudRate baudRate);

/**
 * 启动蓝牙
 */
int app_bt_reset(Device *device);

/**
 * 设置组网ID  "a3a4"
 */
int app_bt_setNetId(Device *device, char *netid);

/**
 * 设置Mac地址  "0101"
 */
int app_bt_setMaddr(Device *device, char *maddr);


#endif // __APP_BT_H
