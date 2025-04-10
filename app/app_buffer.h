#if !defined(__APP_BUFFER_H)
#define __APP_BUFFER_H
#include <pthread.h>

// 内部的子缓冲区
typedef struct 
{
    unsigned char *ptr; // 指向缓冲区的指针
    int total_size; // 缓冲区的大小
    int len; // 缓冲区中数据的长度
} SubBuffer;
// 缓冲区
typedef struct
{
    SubBuffer *sub_buffers[2]; // 2个子缓冲区
    int read_index; // 读缓冲区的索引
    int write_index; // 写缓冲区的索引
    pthread_mutex_t read_lock; // 读锁
    pthread_mutex_t write_lock; // 写锁
} Buffer;

/**
 * 初始化缓冲区
 */
Buffer *app_buffer_init(int size);


/**
 * 释放缓冲区
 */
void app_buffer_free(Buffer *buffer);

/**
 * 向缓冲区中写入数据
 */
int app_buffer_write(Buffer *buffer, char *data, int data_len);

/**
 * 从缓冲区中读取数据
 */
int app_buffer_read(Buffer *buffer, char *data_buf, int buf_size);

#endif // __APP_BUFFER_H
