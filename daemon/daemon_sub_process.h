#if !defined(__DAEMON_SUB_PROCESS_H)
#define __DAEMON_SUB_PROCESS_H

#include <sys/types.h>

#define MAX_FAIL_COUNT 10

// 被守护的子进程结构体
typedef struct
{
    pid_t pid; // 子进程的PID
    char *cmd_param; // 子进程的命令行参数 app/ota
    int fail_count; // 子进程失败退出的次数
} SubProcess;

/**
 * 初始化子进程
 */
SubProcess *daemon_sub_process_init(char *cmd_param);

 /**
  * 检查并启动子进程
  */
int daemon_sub_process_checkStart(SubProcess *sp);

/**
 * 停止子进程
 */
void daemon_sub_process_stop(SubProcess *sp);

#endif // __DAEMON_SUB_PROCESS_H
