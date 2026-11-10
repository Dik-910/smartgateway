#!/bin/sh
#
# 建立虚拟 CAN 总线环境，没有 USB-CAN 硬件也能跑通整条链路。
#
#   sudo ./scripts/vcan_setup.sh          创建并启动 vcan0
#   sudo ./scripts/vcan_setup.sh down     关闭并删除 vcan0
#
# 建好之后把 app_can.h 里的 CAN_DEFAULT_IFNAME 改成 "vcan0"，重新 make 即可。
#
# 注意：vcan 是纯软件总线，不会产生位错误 / CRC 错误 / ACK 错误，
# 所以 **Bus Off 无法在 vcan 上复现**。vcan 只能验证数据通路
# （收帧 -> 上行缓冲区 -> 线程池 -> MQTT），Bus Off 要真实硬件。

set -e

IFNAME="vcan0"

if [ "$1" = "down" ]; then
    ip link set "$IFNAME" down 2>/dev/null || true
    ip link del "$IFNAME" 2>/dev/null || true
    echo "已删除 $IFNAME"
    exit 0
fi

# 1. 加载 vcan 内核模块（已加载会报错，忽略）
modprobe vcan 2>/dev/null || true

# 2. 接口不存在才创建
if ! ip link show "$IFNAME" >/dev/null 2>&1; then
    ip link add dev "$IFNAME" type vcan
    echo "已创建 $IFNAME"
fi

# 3. 启动（等价于真实 CAN 的 ip link set can0 up type can bitrate 500000）
ip link set up "$IFNAME"

echo "--- 接口状态 ---"
ip -details link show "$IFNAME"

echo ""
echo "接下来可以这样验证："
echo "  发一帧:      cansend $IFNAME 123#1122334455667788"
echo "  观察总线:    candump $IFNAME"
echo "  启动网关:     ./gateway app   （CAN_DEFAULT_IFNAME 要改成 vcan0）"
echo ""
echo "cansend / candump 来自 can-utils：apt install can-utils"
