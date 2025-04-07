
# 定义变量 
# 目标名: 前置依赖文件
#	命令1
#	命令2
# objects := main.o hello.o
# objects := main.o \
#			hello.o
objects := main.o
objects += hello.o

# 定义伪目标
.PHONY: clean

main: $(objects)
	gcc $(objects) -o main

hello.o: hello.c
	gcc -c hello.c -o hello.o

main.o: main.c hello.h
	gcc -c main.c -o main.o

clean:
	rm $(objects) main


clean2:
	-rm main.o
	-rm hello.o
	-rm main


