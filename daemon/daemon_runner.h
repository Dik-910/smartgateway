#if !defined(__DAEMON_RUNNER_H)
#define __DAEMON_RUNNER_H

// #define LOG_FILE_PATH "/home/zxf/gateway.log" // 日志文件路径  测试
#define LOG_FILE_PATH "/root/gateway.log"
#define SUB_PROCESS_COUNT 2   // 被守护的子进程数量

int daemon_runner_run();

#endif // __DAEMON_RUNNER_H
