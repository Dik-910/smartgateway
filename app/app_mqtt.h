#if !defined(__APP_MQTT_H)
#define __APP_MQTT_H

#define ADDRESS "ws://192.168.11.91:1883"
#define CLIENTID "b253ba38-daf6-4b37-984f-5d8fdc6a1cfa"
#define TOPIC_R2G "remote_to_gateway" // 接收远程消息的主题
#define TOPIC_G2R "gateway_to_remote" // 发送消息给远程的主题
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L

/**
 * 初始化MQTT客户端
 */
int app_mqtt_init();

/**
 * 关闭MQTT客户端
 */
void app_mqtt_close();

/**
 * 发送消息给远程服务器
 */
int app_mqtt_send(char *json);

/**
 * 注册接收处理远程消息的函数
 */
void app_mqtt_registerRecvCallback(int (*callback)(char *json));

#endif // __APP_MQTT_H
