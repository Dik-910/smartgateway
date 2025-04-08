CC:=gcc
CFLAGS := -g -O0 -Wall

log:=thirdparty/log/log.h thirdparty/log/log.c
log_test: test/log_test.c $(log)
	-gcc $^ -o $@ -Ithirdparty
	-./$@
	-rm $@
json:=thirdparty/cJSON/cJSON.h thirdparty/cJSON/cJSON.c
json_test: test/json_test.c $(json) $(log)
	-$(CC) $(CFLAGS) $^ -o $@ -I thirdparty
#	-./$@
#	-rm $@

app_common:=app/app_common.h app/app_common.c
app_common_test: test/app_common_test.c $(app_common) $(log)
	-$(CC) $^ -o $@ -Iapp -Ithirdparty
	-./$@
	-rm $@

app_message:=app/app_message.h app/app_message.c
app_message_test: test/app_message_test.c $(app_message) $(app_common) $(log) $(json)
	-$(CC) $^ -o $@ -Iapp -Ithirdparty
	-./$@
	-rm $@

# $^: 依赖列表
# $@: 目标名
# -I: 给gcc配置包含的查看路径
# -l: 指定用到的下载的库


mqtt_test: test/mqtt_test.c
	-$(CC) $^ -o $@ -lpaho-mqtt3c
	-./$@
	-rm $@

app_mqtt:=app/app_mqtt.h app/app_mqtt.c
app_mqtt_test: test/app_mqtt_test.c $(app_mqtt) $(log)
	-$(CC) $^ -o $@ -Iapp -Ithirdparty -lpaho-mqtt3c
	-./$@
	-rm $@