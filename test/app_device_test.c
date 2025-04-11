#include "app_device.h"
#include <unistd.h>


int main(int argc, char const *argv[])
{
    app_device_init("/home/zxf/桌面/gateway/serial_file");

    app_device_start();

    sleep(100);

    app_device_close();

    return 0;
}
