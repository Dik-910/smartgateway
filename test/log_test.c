// #include "../thirdparty/log/log.h"
#include "log/log.h"

int main(int argc, char const *argv[])
{
    // 设置日志输出级别
    log_set_level(LOG_DEBUG);

    //  做不同级别的日志输出
    log_trace("trace log");
    log_debug("debug log");
    log_info("info log");
    log_warn("warn log");
    log_error("error log");
    log_fatal("fatal log");

    return 0;
}
