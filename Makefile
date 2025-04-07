log:=thirdparty/log/log.h thirdparty/log/log.c
log_test: test/log_test.c $(log)
	-gcc $^ -o $@ -Ithirdparty
	-./$@
	-rm $@

# $^: 依赖列表
# $@: 目标名
# -I: 给gcc配置包含的查看路径