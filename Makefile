CC:=$(CROSS_COMPILE)gcc

BOARD_DIR := $(shell pwd)/
PEER := root@192.168.11.93

CFLAGS += -Wall -Wextra

CFLAGS += -I.
CFLAGS += -Ithirdparty
CFLAGS += -Iapp
CFLAGS += -Idaemon
CFLAGS += -Iota

ifdef SYSROOT
	CFLAGS += --sysroot=$(SYSROOT)
endif

LDLIBS += -lpaho-mqtt3c
LDLIBS += -lcurl
LDLIBS += -lcrypto

SRC += $(shell find app -name "*.c"	-type f)
SRC += $(shell find daemon -name "*.c"	-type f)
SRC += $(shell find ota -name "*.c"	-type f)
SRC += $(shell find thirdparty -name "*.c"	-type f)

OBJ := $(SRC:.c=.o)

TARGET := gateway

.PHONY: all, clean

all: $(TARGET)

clean:
	@-rm -f $(TARGET) $(OBJ) main.o

$(TARGET): main.o $(OBJ)
	@-$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

cross-compile:
	@CROSS_COMPILE=$(BOARD_DIR)/toolchain/bin/arm-linux-gnueabihf- \
	 SYSROOT=$(BOARD_DIR)/sysroot \
	 make -j16
	@scp -O $(TARGET) $(PEER):/usr/bin/$(TARGET)

cross-init:
	@scp -O init/S99gateway $(PEER):/etc/init.d/S99gateway

%.o: %.c
	@-$(CC) $(CFLAGS) -c $^ -o $@

# ---- 单元测试 ----
# 纯逻辑单测只链「被测模块 + 它依赖的通用模块」，不能链 $(OBJ)：
# $(OBJ) 里含着 ota_http.o（要 libcurl）和 ota_version.o（要 openssl），
# 单测一个也用不到，却会因为缺 dev 包而链接失败，把真正的测试结果盖掉。
CAN_UNIT_DEPS := app/app_can.c thirdparty/log/log.c

# 去掉了原来的 @- 前缀：加 @- 会让编译失败、运行失败统统被 make 吞掉，
# 结果「测试压根没跑起来」和「测试 34 项全过」在终端上长得一模一样。
%_test: test/%_test.c $(CAN_UNIT_DEPS)
	$(CC) $(CFLAGS) $^ -o $@
	./$@