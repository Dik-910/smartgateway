#if !defined(__APP_BT_H)
#define __APP_BT_H

#include "app_device.h"

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

#endif // __APP_BT_H
