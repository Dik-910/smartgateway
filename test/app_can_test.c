#include "app_can.h"
#include <linux/can/error.h>
#include <string.h>
#include <stdio.h>

/**
 * app_can 转换层的单元测试
 *
 * 只测纯逻辑（can_frame <-> 字符数组消息），不碰 socket，
 * 所以不依赖 vcan 环境，make app_can_test 就能跑。
 */

static int passed = 0;
static int failed = 0;

static void check(int cond, const char *what)
{
    if (cond)
    {
        printf("  [PASS] %s\n", what);
        passed++;
    }
    else
    {
        printf("  [FAIL] %s\n", what);
        failed++;
    }
}

// 标准帧：11 位 ID + 3 字节数据
static void test_std_frame(void)
{
    printf("标准帧往返 0x123 dlc=3\n");

    struct can_frame src;
    memset(&src, 0, sizeof(src));
    src.can_id = 0x123;
    src.can_dlc = 3;
    src.data[0] = 0x11;
    src.data[1] = 0x80; // 高位字节，验证不会符号扩展
    src.data[2] = 0xFF;

    char chars[CAN_CHARS_MAX_LEN];
    int len = app_can_frame2Chars(&src, chars, sizeof(chars));

    check(len == 3 + CAN_STD_ID_LEN + 3, "消息长度 = 3 + 2 + 3 = 8");
    check((unsigned char)chars[0] == CONN_TYPE_CAN, "conn_type = 2");
    check((unsigned char)chars[1] == CAN_STD_ID_LEN, "id_len = 2");
    check((unsigned char)chars[2] == 3, "msg_len = 3");
    check((unsigned char)chars[3] == 0x01 && (unsigned char)chars[4] == 0x23, "ID 大端 = 01 23");
    check((unsigned char)chars[5] == 0x11 && (unsigned char)chars[6] == 0x80
              && (unsigned char)chars[7] == 0xFF, "数据原样 11 80 FF");

    struct can_frame dst;
    int ret = app_can_chars2Frame(chars, len, &dst);
    check(ret == 0, "消息转回帧成功");
    check(dst.can_id == 0x123, "ID 还原 = 0x123");
    check(dst.can_dlc == 3, "dlc 还原 = 3");
    check(memcmp(dst.data, src.data, 3) == 0, "数据还原一致");
}

// 扩展帧：29 位 ID + 8 字节数据
static void test_ext_frame(void)
{
    printf("扩展帧往返 0x18FF1234 dlc=8\n");

    struct can_frame src;
    memset(&src, 0, sizeof(src));
    src.can_id = 0x18FF1234 | CAN_EFF_FLAG;
    src.can_dlc = 8;
    for (int i = 0; i < 8; i++)
    {
        src.data[i] = (unsigned char)(0xF0 + i);
    }

    char chars[CAN_CHARS_MAX_LEN];
    int len = app_can_frame2Chars(&src, chars, sizeof(chars));

    check(len == 3 + CAN_EXT_ID_LEN + 8, "消息长度 = 3 + 4 + 8 = 15");
    check((unsigned char)chars[1] == CAN_EXT_ID_LEN, "id_len = 4");
    check((unsigned char)chars[3] == 0x18 && (unsigned char)chars[4] == 0xFF
              && (unsigned char)chars[5] == 0x12 && (unsigned char)chars[6] == 0x34,
          "ID 大端 = 18 FF 12 34（标志位已剥掉）");

    struct can_frame dst;
    int ret = app_can_chars2Frame(chars, len, &dst);
    check(ret == 0, "消息转回帧成功");
    check((dst.can_id & CAN_EFF_FLAG) != 0, "扩展帧标志位被置起");
    check((dst.can_id & CAN_EFF_MASK) == 0x18FF1234, "ID 还原 = 0x18FF1234");
    check(memcmp(dst.data, src.data, 8) == 0, "8 字节数据还原一致");
}

// 零长度数据帧（比如纯粹的心跳/唤醒帧）
static void test_empty_data(void)
{
    printf("空数据帧 0x7FF dlc=0\n");

    struct can_frame src;
    memset(&src, 0, sizeof(src));
    src.can_id = 0x7FF;
    src.can_dlc = 0;

    char chars[CAN_CHARS_MAX_LEN];
    int len = app_can_frame2Chars(&src, chars, sizeof(chars));
    check(len == 5, "消息长度 = 3 + 2 + 0 = 5");

    struct can_frame dst;
    check(app_can_chars2Frame(chars, len, &dst) == 0, "消息转回帧成功");
    check(dst.can_id == 0x7FF && dst.can_dlc == 0, "ID/dlc 还原一致");
}

// 非法输入必须被挡住，不能悄悄生成一帧垃圾丢上总线
static void test_invalid(void)
{
    printf("非法输入拦截\n");

    char chars[CAN_CHARS_MAX_LEN];
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    // conn_type 不是 CAN
    chars[0] = 1; chars[1] = 2; chars[2] = 1; chars[3] = 0; chars[4] = 1; chars[5] = 0;
    check(app_can_chars2Frame(chars, 6, &frame) == -1, "conn_type=1 被拒绝");

    // ID 超出 11 位上限
    chars[0] = CONN_TYPE_CAN; chars[1] = 2; chars[2] = 0;
    chars[3] = 0x08; chars[4] = 0x00; // 0x0800 > 0x7FF
    check(app_can_chars2Frame(chars, 5, &frame) == -1, "标准帧 ID 0x800 被拒绝");

    // 数据长度超过 8
    chars[0] = CONN_TYPE_CAN; chars[1] = 2; chars[2] = 9;
    check(app_can_chars2Frame(chars, 3 + 2 + 9, &frame) == -1, "数据长度 9 被拒绝");

    // 声明长度和实际长度对不上
    chars[0] = CONN_TYPE_CAN; chars[1] = 2; chars[2] = 4;
    check(app_can_chars2Frame(chars, 3 + 2 + 2, &frame) == -1, "长度不一致被拒绝");

    // 错误帧不能当数据帧转换
    struct can_frame err;
    memset(&err, 0, sizeof(err));
    err.can_id = CAN_ERR_FLAG | CAN_ERR_BUSOFF;
    char out[CAN_CHARS_MAX_LEN];
    check(app_can_frame2Chars(&err, out, sizeof(out)) == -1, "错误帧被拒绝");

    // 容器太小
    struct can_frame big;
    memset(&big, 0, sizeof(big));
    big.can_id = 0x123;
    big.can_dlc = 8;
    check(app_can_frame2Chars(&big, out, 4) == -1, "容器不足被拒绝");
}

// 错误帧上报：Bus Off 与带 TEC/REC 计数的错误帧
static void test_error_frame(void)
{
    printf("错误帧上报\n");

    char chars[CAN_CHARS_MAX_LEN];

    // Bus Off：节点 TEC 超过 255，已经脱离总线
    struct can_frame busoff;
    memset(&busoff, 0, sizeof(busoff));
    busoff.can_id = CAN_ERR_FLAG | CAN_ERR_BUSOFF;
    int len = app_can_err2Chars(&busoff, chars, sizeof(chars));
    check(len == 9, "Bus Off 消息长度 = 3 + 2 + 4 = 9");
    check((unsigned char)chars[0] == CONN_TYPE_CAN_EVT, "conn_type = 3（总线事件）");
    check((unsigned char)chars[3] == 0x00 && (unsigned char)chars[4] == 0x40,
          "事件码 = 0x0040（CAN_ERR_BUSOFF）");
    // TEC 2 字节大端：0x0100 = 256
    check((unsigned char)chars[5] == 0x01 && (unsigned char)chars[6] == 0x00,
          "TEC 上报为 0x0100 = 256（超过 255，一字节装不下）");

    // 带计数器的普通总线错误：CAN_ERR_CNT 置位时 data[6]=TEC data[7]=REC
    struct can_frame buserr;
    memset(&buserr, 0, sizeof(buserr));
    buserr.can_id = CAN_ERR_FLAG | CAN_ERR_CNT | CAN_ERR_CRTL;
    buserr.data[6] = 130; // 已进入错误被动态（TEC > 127）
    buserr.data[7] = 5;
    check(app_can_err2Chars(&buserr, chars, sizeof(chars)) == 9, "带计数的错误帧转换成功");
    check((unsigned char)chars[5] == 0 && (unsigned char)chars[6] == 130,
          "TEC 还原 = 130（错误被动）");
    check((unsigned char)chars[7] == 0 && (unsigned char)chars[8] == 5,
          "REC 还原 = 5");

    // 普通数据帧不能当错误帧上报
    struct can_frame data;
    memset(&data, 0, sizeof(data));
    data.can_id = 0x123;
    check(app_can_err2Chars(&data, chars, sizeof(chars)) == -1, "普通数据帧被拒绝");
}

int main(int argc, char const *argv[])
{
    test_std_frame();
    test_ext_frame();
    test_empty_data();
    test_invalid();
    test_error_frame();

    printf("\n结果：%d 通过，%d 失败\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
