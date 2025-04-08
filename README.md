### 智能网关

```
- 什么？
	使用特定语言（比如：C）实现的一套程序应用，
	让不能联网的设备能间接连接上网络，实现与远程客户端的双向通信
- 作用？
	让多个不同设备A（不能联网），借助“智能网关”及其所在的本地设备B（能联网），
	实现与远程服务器和远程设备C（能联网）进行实时双向通信。
```

### 几个概念

```
- Unix: 多用户、多任务的非开源操作系统    
- POSIX： 是一组标准，定义了UNIX和类UNIX操作系统和工具的行为规范   API
- GNU：（GNU's Not Unix）一个完全自由的类Unix的操作系统，包括：bash/gcc/gdb/glibc等
- glibc：GNU 提供的标准 C 库实现，包含了大量符合 POSIX 标准的函数。
- gcc: C/C++等语言代码的跨平台的编译器
- GNU C:  GNU 的 C 编译器（gcc）以及相关的库(glibc)和工具的统称。
```

### 创建项目目录

```
app：应用主功能程序
daemon：守护进程程序
init：开机自启动shell脚本配置
ota：在线升级程序
test：测试程序
thirdparty：第三方工具包
.gitignore：git的忽略配置
main.c：应用入口主程序
Makefile：自动化构建make配置
```

### make

```
- 理解：
	Make是一个自动化构建工具，用于从源代码到构建程序、运行和删除文件等操作 
	我们需要在Makefile文件中定义不同的目标来执行不同的操作

- 下载包含Make的工具包：
	sudo apt install -y make

- 编写Makefile文件
	定义目标
		目标名称：前置依赖（目标/文件）
			命令1
			命令2
	说明	
		目标一般都指定为要生成的同名文件
		在执行某个目标的命令前，会先执行前置依赖目标，生成依赖文件

- 定义变量
	objects := hello.o main.o
	objects := hello.o \
			   main.o
	object += xxx.o
	$(objects)

- 定义伪目标
	为什么要定义：防止有同名文件后，目标总是不能再执行（对应的文件已有了）
	哪些目标要定义：不生成对应文件的目标
	.PHONY= target1,target2

- 问题：为什么要指定依赖文件
	理解：如果不指定依赖文件，如果目标文件已存在，但依赖文件发生了改变，再次执行目标，不重新命令
	当然：如果目标文件和依赖文件都没有发生变化，重新执行目标肯定不再执行命令了

$^: 代表依赖列表
$@：目标名称
-I: 指定include path
```



### 使用git管理项目

```
- 配置git忽略
	.gitignore
- 创建本地仓库
	git init   初始仓库生成工作区
	git add .  将工作区的代码添加到暂存区
	git commit -m "xxx"  将暂存区的代码更新提交到本地仓库（版本区）
- 创建远程仓库
	注册/登陆gitee账号 =》 创建开源远程仓库
- 将本地仓库推送到远程仓库
	git remote add origin 远程仓库的地址
	git push origin master
- 修改工作区代码，先提交到本地仓库，再推送到远程仓库
	git add .
	git commit -m "yyy"
	git push origin master
- 从https切换到SSH
	生成公钥与私钥，并将公钥配置到远程： ssh-keygen -t rsa
	先删除https的origin, 再添加ssh的origin:
		git remote remove origin
		git remote add origin ssh的url
		git remote -v 查看远程仓库
	推送仓库(第一次)
		git push origin master
		选择输入yes
```

### 使用第三方库：log.c

```
- 基本语法：
  log_xxx()  多个级别的日志输出
  log_set_level(level)
- 引入库的源文件：log.c/log.h
- 编写测试程序
- 在vscode中将thirdparty配置为include path
	设置 =》 搜索 include => 在include path中添加thirdparty目录
- 在vscode中将C语言的核心库路径配置到include path
	添加/usr/include/**    /usr/lib/gcc/**
- 在Makefile中告诉gcc将thirdparty作为库目录：给gcc命令添加 -Ithirdparty
- 利用Makefile编译运行测试
```

### json理解

```
特定格式的字符串
{}： json对象
[]： json数组
{key1:value1, key2:value2}
[value1, value2]
key的类型：字符串   必须用双引号包含
value的类型： 字符串，数值，true/false, null, [], {}

{"name":"tom"，"age":12}  // 未格式化
[{"name":"tom","age":12}, {"name":"tom2","age":13}]
{
	"name":"tom",
	"age":12
} // 格式化的json
```

### 使用第三方库：cJSON

```
json数据解析和生成的c语言库
将cJSON的cJSON.h和cJSON.c拷贝到工程的thirdparty/cJSON目录下
操作1：解析JSON字符串后，读取出其中特定属性的数据
操作2：生成一个包含特定属性的JSON字符串
```

### 断点调试

```
- 下载插件：makefile tools
- 配置Makefile： 针对某个测试目标开启调试，只编译程序，不直接运行程序和删除程序
- 给插件配置要生成和运行的目标
- 打断点
- 通过插件来debug运行
- 进行调试操作
```

### 通用工具模块：包含一些多个模块需要的工具函数或宏定义

```
- app/app_common.h  & app_common.c
- app_common_getCurrentTime(): 获取当前时间戳（毫秒值）
- 测试：
  test/app_common_test.c
  配置Makefile并运行
```

### 消息模块

```
- 消息数据的组成：
	连接类型： 1 代表蓝牙连接
	设备id：“XX"
	消息体: "abcd"
- 消息数据的2种格式   
	字符数组:  1 2 4 XXabcd
	json字符串: {"conn_type": 1, "id": "5858", "msg": "61626364"}
- 字符数组格式的消息
	字节1：连接类型
	字节2：设备id长度
    字节3：消息体长度
    字节4~n：设备id和消息体
- json字符串格式的消息
	conn_type: 连接类型
	id:下游设备的id
	msg：消息体
	注意：id和msg的值是十六进制字符串
- 字符数组与十六进制字符串的相互转换
	长度关系：2倍
	chars => hexstr: sprintf(hex_char_pointer, "%02X", char)
	hexstr => chars: sscanf(hex_char_pointer, "%02X", char_pointer)
	可以在app_common中定义好转换的工具函数
		app_common_charsToHexstr
		app_common_hexstrToChars
- 2种格式消息的相互转换
	应用消息模块： app_message
	app_message_charsToJson: 字符数组格式转为json格式
	app_message_jsonToChars：json格式转为字符数组格式
- 测试
```

### MQTT协议

```
- 理解MQTT
	轻量级的基于发布/订阅模式的通信协议， 在物联网和传感器应用广泛使用
- 搭建MQTT服务器（Mosquitto）
	安装： windows版本
	配置：mosquitto.conf
		指定端口号和任意IP的客户端都可以访问
		指定可以匿名访问
	重启电脑，自动启动服务
- 使用LLCOM工具进行测试
	测试是否能连接
	测试订阅与发布主题
- 开发MQTT客户端应用测试
	下载客户端库：libpaho-mqtt-dev
	根据文档编写测试代码
		创建客户端: create
		设置回调: setCallbacks
		连接  connect
		订阅主题  subscribe
		发布主题  publishMessage
		断开连接与销毁 disconnect / destroy
```

### MQTT客户端模块

```
- 在app_mqtt.h中定义相关函数
	app_mqtt_init()
	app_mqtt_close()
	app_mqtt_send(char *json)
	app_mqtt_registerRecvCallback(int callback(char *json))
- 在app_mqtt.c中利用客户端库实现各个函数
```



### 进程与线程

```
- 进程
	正在运行的程序，是系统的一个基本执行单元
	每个进程都有自己独立的内存空间（代码、数据、堆、栈、文件描述符等资源）
- 线程
	进程内的一个更小的执行单元
	每个线程都有自己的线程函数
	每个线程有自己独立的调用栈空间，但共享同一个进程内的其它资源
- 多线程比多进程更适合高并发：线程更轻易级，占用空间更小，运行消耗更小，更快些
- 一个程序启动运行：
	至少要启动一个进程：主进程，当然也可以进而启动n个子进程
	主进程中至少要启动一个线程：主线程，当然也可以进而启动n个分线程
	main函数是在主进程的主线程上执行的，其它函数可以在其它线程上执行
- 启动多个进程或线程执行
	执行的先后顺序不是完全固定的，由内核调度执行，
	执行过程中，可能会被暂停，切换到执行另一个进程或线程
```

### 编码控制线程

```
1) 创建线程
- pthread_create()
	作用：创建一个线程
	参数1：线程标识符的指针,线程创建成功时,用于存储新创建线程的线程标识符
	参数2：pthead_attr_t结构体,这个参数可以用来设置线程的属性,如优先级、栈大小等。
      		如果不需要定制线程属性,可以传入 NULL,此时线程将采用默认属性。
	参数3：新线程函数的指针
	参数4：函数的参数,可以是一个指向任意类型数据的指针
	返回值：成功：0,失败非0

2) 终止线程
- 方式一：线程函数执行return语句
- 方式二：线程函数内部调用pthread_exit函数
- 方式三：其他线程调用pthread_cancel函数
- 一旦main线程结束，进程也结束，如果还有线程未执行完就不可能再执行了
- 一个线程等待另一个线程结束：pthread_join()，经常用于main线程等待其它线程结束

3）线程同步
- 多线程并发执行：多个线程同时执行，且很可能操作同一份数据，且一个线程执行过程中可能被系统暂停切换到另一个线程执行
- 线程同步：多个线程共享操作同一份数据，不要出现一个线程对数据进行读写的中间被切换到另一个线程执行的情况，另一种就是控制线程顺序执行。
- 竞态条件：是一种特定的线程同步问题，指的多个进程或者线程并发执行时，其最终的结果依赖于进程或者线程执行的精确时序，它可能会导致执行结果不对。
- 避免竞态条件的最常见办法：给资源加锁，某一时间段内只能有一个线程操作数据 =》互斥锁

4）互斥锁
- 互斥锁是一种同步机制，用来控制对共享资源的访问。在任何时刻，最多只能有一个线程持有特定的互斥锁。
   如果一个线程试图获取一个已经被其他线程持有的锁，那么请求锁的线程将被阻塞，直到锁被释放。
- 互斥锁的操作
	初始化（PTHREAD_MUTEX_INITIALIZER）
	锁定（pthread_mutex_lock()）：获取锁，如果此时锁被占则阻塞，否则函数返回，向下执行
	解锁（pthread_mutex_unlock()）：释放锁，获取锁而阻塞的线程就有机会获取到锁而向下执行
	销毁（pthread_mutex_destroy()）：释放锁占用的所有内存和资源
```

### 消息队列

```
- 理解：是一种进程/线程间通信机制，允许进程/线程以消息的形式进行异步交换数据
- 相关操作及API
	创建或打开消息队列: mq_open()
	发送消息: mq_send() 
	接收消息: mq_receive()
    关闭消息队列: mq_close()
    删除消息队列: mq_unlink()
```

### 线程池模块

```
- 线程池：
	管理和重用多个线程的设计模式
	让多个线程可以不断的执行，而不是执行完一个任务后就销毁，提高性能
- 线程模块
	多线程异步处理其它模块注册的多个待执行的任务
	线程池 + 消息队列
	创建消息（任务）队列：存储待执行的N个任务（函数和参数数据）
	创建N个线程，在线程函数中循环取出任务队列中的任务执行，如果队列中没有任务，就进入阻塞状态
	其它模块可以向线程模块注册任务，也就是发送到队列中
- app_pool
	struct Task
	app_pool_init()
	app_pool_close()
	app_pool_registerTask()
```

### 缓冲模块

```
- 区别缓存（Cache）与缓冲（Buffer）
	都是开辟一段内存，在其中临时存储一些数据
	缓存
		在内存中存储一份最近要高频使用的数据，以提高反复读取数据的速度
	缓冲
		主要为了平衡数据传输速度不匹配的问题
		一方面不断将多份（帧）数据快速保存到缓冲区内存中（快），另一方面利用多线程来不断读取缓冲区的各个帧数据进行处理（慢）。
		
- 理解双缓冲机制
	利用缓冲区来存储多帧数据，向缓冲区写数据和从缓存区数据都是多线程执行的，为了数据安全我们会对读写操作进行加锁操作，以防止出现竞态条件。
	单缓冲（环形缓冲）：
		一个缓冲内存，那样读和写操作的是同一块空间，那就意味着读时不能写，写时也不能读
		缺点：速度慢，优点：占用空间小
		适用：存储空间有限，对速度要求不高的设备
	双缓冲：
		二个缓冲内存，一块内存写数据，另一块内存读数据，如果读时发现没有，切换读写内存
		优点：速度快，缺点：占用空间大
		适用：存储空间足够，对速度有一定要求的设备
		
一个缓冲区包含2个存储数据的小缓冲区（一个用于读，另一个用于写）
每个小/子缓冲  SubBuffer：
	char *ptr
	int total_size
	int len
整个缓冲区： Buffer
	SubBuffer *sub_buffers[2]
	int read_index = 1
	int write_index = 0
	pthread_mutex_t read_lock
	pthread_mutex_t write_lock

- app_buffer.h
	小缓冲区结构体
		unsigned char *ptr; // 数据指针
        int total_size; // 空间大小
        int len; // 数据长度
	大缓冲区结构体
		SubBuffer *sub_buffers[2]; // 读和写缓冲区的数组
        int read_index; // 读缓冲区索引
        int write_index; // 写缓冲区索引
        pthread_mutex_t read_lock; // 读锁
        pthread_mutex_t write_lock; // 写锁
    功能函数
    	app_buffer_init(): 初始化缓冲区
    	app_buffer_free(): 释放缓冲区
    	app_buffer_write(): 向缓冲区写入数据
    	app_buffer_read(): 从缓冲区读取数据
    	
- app_buffer.c
	写入数据时：
		写入数据帧前需要限制被写入的数据帧的长度不要超过255
		原因：我们在写入数据帧时，会将其长度用一个字节的空间保存在数据帧的帧头 =》方便读取
	读取数据时：
		在读取前，如果读缓冲区为空，切换缓冲区
		在读完数据后，如果读缓冲区为空，也切换缓冲区
	加锁处理
		在读缓冲区的数据前后需要进行加锁和解锁   加读锁 =》 不能同时有2个线程读
		在写缓冲区的数据前后需要进行加锁和解锁   加写锁 =》 不能同时有2个线程写
		在切换缓冲区前后需要进行加锁和解锁       加写锁 =》 写入时，不能切换

```

### 系统调用IO

```
- 区别系统调用与标准库函数
	系统调用： 
		也是函数，能直接与系统内核交互，能直接操作底层文件
		比如：open, write, read, close
	库函数：
		一部分是对系统调用的包装，也能直接与内核交互
		比如：fopen, fwrite, fread, fclose
		一部分没有用到系统调用，不能直接与内存交互
		比如：strlen
	网关项目中的文件操作：使用系统调用IO

- 标准输入输出
	标准输入输出流 （File *）
        FILE *stdin;   标准输入
        FILE *stdout;  标准输出
        FILE *stderr;  标准错误输出
    标准输出输出流的文件描述符
        STDIN_FILENO	0  标准输入
        STDOUT_FILENO	1  标准输出
        STDERR_FILENO	2  标准错误输出

- 标准输出重定向
	关闭标准输出和标准错误输出： close()
	将标准输出重定向到特定正常日志文件: open()  1
	将标准错误输出重定向到特定错误日志文件: open()  2
```

### 设备模块

```
- 整个项目最核心模块，依赖(使用)了前面完成的模块
	工具模块        app_common
	消息模块		app_message
	MQTT客户端模块  app_mqtt
	线程池模块	   app_pool
	缓冲模块		app_buffer
- 主要包含2个方向的数据流操作
	上行：下游设备 =》网关 =》远程服务器/用户设备(手机)
	下行：远程用户设备/服务器 =》网关 =》下游设备
- 需要通过分线程执行的操作
	不断读取下游设备传输过来的数据 =》 创建一个线程不断从对应的串口文件中读取 
	上行过程中发送N个消息数据到远程 =》 线程池
	下行过程中MQTT接收远程消息的回调函数 =》 分线程
	下行过程中将接收的N个消息数据写入串口文件中 =》 线程池
- 上行程序操作流程
	启动读取下游设备线程的线程，在线程函数中  read_thread_func     
		不断读取下流设备通过蓝牙连接传输过来的数据  read(fd)
		对蓝牙数据进行处理转换为字符数组消息  post_read() 
		将字符数组消息保存到上行缓冲区		app_buffer_write(up_buffer)
		将发消息数据到远程的任务交给线程池执行   app_pool_registerTask(send_task_func)
	在任务函数中  send_task_func
		从上行缓冲中读取出一个字符数组消息	app_buffer_read(up_buffer)
		将字符数组消息转换为json格式消息	  app_message_charsToJson()
		将json消息发送给远程			  app_mqtt_send(json)
- 下行程序操作流程
	注册接收远程消息的回调函数  app_mqtt_registerRecvCallback(receive_msg_callback)
		将json格式消息转换为字符数组消息 app_message_jsonToChars()
		将其保存到下行缓冲中  app_buffer_write(down_buffer)
		将消息数据保存到设备文件的任务交给线程池执行 app_pool_regiserTask(write_task_func)
	在任务函数中 write_task_func
		从下行缓冲中读取出一个字符数组消息  app_buffer_read(down_buffer)
		将字符数组消息转换为蓝牙需要的格式数据  pre_write()
		将数据保存到蓝牙对应的串口文件中，传递给下游设备 write(fd)

- 设备结构体
	typedef struct
    {
        char *filename; // 设备文件 接收了下游设备发过来数据的文件
        int fd; // 文件描述符
        Buffer *up_buffer; // 上行缓冲区
        Buffer *down_buffer; // 下行缓冲区
        pthread_t read_thread; // 读设备数据的线程
        int is_running; // 读线程是否运行 0 / 1
        long last_write_time; // 上次写数据的时间
        // 从设备读取数据后，对数据的处理函数，将数据处理成字符数组消息
        int (*post_read)(char *data, int len); 
        // 写数据前，对数据的处理函数，将字符数组消息处理成设备需要的格式
        int (*pre_write)(char *data, int len); 
    } Device;
```

### 蓝牙模块的preWrite与postRead

```
- 向蓝牙写入数据前的处理  preWrite
    字符数组消息: 
        例子：1 2 3 XX abc
        格式：conn_type id_len msg_len id msg
    蓝牙发送数据格式例子：
        例子：41 54 2b 4d 45 53 48 00 ff ff 61 62 63 0d 0a
        41 54 2b 4d 45 53 48 00： AT+MESH（固定头部）
        ff ff: 对端的MADDR（如果是FFFF代表群发）
        61 62 63: 要发送的数据（不超过12字节）
        0d 0a：\r\n（固定结尾）
- 从蓝牙读取到数据后的处理 postRead
    接收方得到数据（3 + [2]）：f1 dd 07 23 23 ff ff 41 42 43
        f1 dd : 固定的头部
        07： 之后数据的长度（5-16之间） 
        23 23：对端（发送方）的MADDR
        ff ff: 我的MADDR或ffff(群发)
        41 42 43：发送的数据 
    处理后的数据格式：conn_type id_len msg_len id msg

- preWrite
	计算出蓝牙数据的长度，并创建临时存储蓝牙数据的空数组
	向数组中依次写入：固定头部、id、msg和固定结尾
	将蓝牙数据拷贝到传入的data中
	返回蓝牙数据长度
- postRead
	思路：一份完整的蓝牙数据可能是需要多次读取才能得到的，所有需要将多个读取的数据合并保存到一个缓存容器中，再从中提取出一个完整结构的蓝牙数据，再生成对应的字符数组消息数据
	将data写入到缓存数组中，与前面的数据合并在一起
	遍历缓存数组
        如果找到的是以f1dd开头的蓝牙数据，
        	删除已遍历的数据
        	取出各个数据，以字符数组消息格式保存到data中
        	将数据从缓存中删除
        	返回字符数组的长度
	遍历结束，删除缓存中已遍历的数据
```

### 串口模块

```
- termios库：linux系统中一个用来操作终端设备的标准接口库（linux中串口也是终端）
	struct termios: 包含串口相关属性的结构体
	tcgetattr(fd, *termios): 获取属性
	tcsetaatr(fd, TCSAFLUSH, *termios): 设置属性，但暂时不生效，需要flush时生效
	cfsetspeed(*termios, speed_t): 设备输入和输出的波特率
	cfmakeraw（*termios）: 将串口终端设置为原始模式
	tcflush(fd, TCIOFLUSH)：flush串口，让设置的属性生效
	
	设置校验位：
		parity = 0; // 无校验位
    	parity = PARENB | PARODD  // 奇校验位
    	parity = PARENB // 偶校验位
		termios.c_cflag &= ~(PARENB| PARODD); // 清除校验位
    	termios.c_cflag |= parity; // 指定校验位
    设置停止位：
    	stop_bits = 0, // 1位停止位
   		stop_bits = CSTOPB, // 2位停止位
    	termios.c_cflag &= ~CSTOPB; // 清除停止位
    	termios.c_cflag |= stop_bits; // 设置停止位
    设置阻塞模式：
    	termios.c_cc[VMIN] = 1; // 至少读取一个字符才返回
    	termios.c_cc[VTIME] = 0; // 没有读到数据会一直等待  
    设置非阻塞模式：
    	termios.c_cc[VMIN] = 0; // 最少读取字符数为0
    	termios.c_cc[VTIME] = 2; // 单位为100ms 等待时间为0.2秒
        
- 串口模块
	enum BraudRate // 波特率枚举
	enum Parity // 校验位枚举
	enum StopBits // 停止位枚举
	app_serial_init()：初始化
	app_serial_setBraudRate()：设置波特率
	app_serial_setParity()：设置校验位
	app_serial_setStopBits()：设置停止位
	app_serial_setBlock(): 设置串口是否是阻塞模式
	app_serial_setRaw(): 设置串口解析数据模式为原始模式：不解析特殊字符
```

### 蓝牙模块

```
- 蓝牙 Mesh：
	是一种基于蓝牙技术的网络协议
	支持多对多设备通信，适用于需要大量设备协同工作的场景，如智能家居、工业自动化和商业照明等
	蓝牙信号也是广播的（与LORA一样）
	多个需要相互通信的蓝牙设备指定相同的NETID来组成一个网络，每个设备就是网络的一个节点，他们可以相互通信，一个网络中的每个节点的MADDR要不同

- JDY-24M蓝牙通信：
	初始化指令：
		AT 测试蓝牙是否可用
        AT+NAMEatguigu 修改蓝牙名称
        AT+BAUD4：设置波特率为9600
        AT+BAUD8 设置波特率为115200
        AT+RESET 重置(修改的配置才能生效) 
    MESH初始化指令：
    	AT+NETID<Param1>  需要相互通信的蓝牙设备的NETID一样
		AT+MADDR<Param1> 同个网络中的各个蓝牙设备的MADDR要不同
	MESH通信指令：
		AT+MESH<param>
	注意：
		发送指令: 最后2位总是\r\n
		操作成功返回：OK\r\n
- 蓝牙模块
	enum BTBaudRate 蓝牙波特率枚举
	app_bt_status() 判断蓝牙是否连接
	app_bt_rename() 蓝牙重命名
	app_bt_setBraudRate() 设置蓝牙波特率
	app_bt_reset() 重启蓝牙
	app_bt_setNetid() 设置蓝牙网络ID
	app_bt_setMaddr() 设置蓝牙MAC地址
	app_bt_init() 初始化蓝牙
	app_bt_postRead() 从蓝牙读取数据后处理函数
	app_bt_preWrite() 写入蓝牙数据前处理函数
```





### 进程

```
- 区别程序与进程
	- 程序是存储在硬盘上的一段二进制序列，是静态的
	- 进程（Process）是正在运行的程序， 是动态的， 包括内存中的代码、数据内存以及分配给它的其他系统资源
- 区别进程与线程
    - 都是Linux系统可以进行调度执行的执行单元
    - 进程是一个运行的程序，有独立私有的内存空间和资源
    - 线程是进程内的一个执行单元，一个进程内的所有线程共享进程的空间和资源
    - 线程间通信更简单，一个全局变量就搞定，但带来的线程同步问题
    - 线程间切换相比进程间切换消耗更小，更适合做高并发处理。
- 理解进程内存模型（结构）
	文本（程序代码： 程序代码、指令
	初始化数据：已经初始化的全局变量和静态变量
	未初始化数据：没有初始化的全局变量和静态变量
	堆（heap）：malloc动态分配
	未分配内存：供堆和栈扩展使用的区域
	栈：局部变量、函数参数、返回地址等
	参数与环境： 程序参数和环境变量

	内核：内核空间，系统内部使用，无法直接访问，需要通过系统调用访问
```

### 操作进程的相关语法

```
- fork()
作用：根据当前进程，创建一个新的子进程
返回值：pid_t 本质是int类型
    失败：-1
    进入子（新的）进程执行： 0
    进入父（当前）进程执行： 子进程的pid
注意：
    创建子进程成功后，fork函数会返回2次，一次是在主进程中执行，一次是在子进程中执行  
    先执行哪个进程不是固定的，由系统内核调度决定
    子进程不会再执行fork之前的代码
    子进程会拷贝一份父进程的内存，所以fork之前声明的变量在子进程也有一份

- execve()
  作用：在当前进程执行指定的程序
  参数1：可执行文件的路径（自定义的或系统的）
  参数2：包含执行文件路径、要传递的参数的字符串数组 (最后一个元素必须是NULL)
  参数3：包含PATH环境变量的字符串数组或NULL（最后一个元素必须是NULL）
  返回值
       失败：-1
       成功：函数不返回，后面的代码不会执行

-waitpid(pid, &status, 0|WNOHANG) 、 wait(null)
 作用：等待指定子进程执行完后再接着执行当前进程
 参数1：子进程id
 参数2：接收子进程执行返回int值结果的指针，如果子进程正常执行完，值将为0,否则是其它值
 参数3：用来指定wait特点的特定数值
	WNOHANG(1): 当前进程不阻塞，不会等待目标进程执行完当前进程就继续向下执行，得不目标进程返回的结果
    WUNTRACED(2)：会等待目标进程执行完当前进程才向下执行，且能得到子进程的返回结果
	0: 效果赞同于WUNTRACED， 一般直接写0

```

### 进程间通信之信号

```
- 信号是一种用于通知进程发生了某种事件的机制
- 信号可以由内核、其他进程或者通过命令行工具发送给目标进程
- 常见的信号
	Ctrl+C =》 发送SIGINT信号
	kill 进程 =》 发送SIGTERM信号
	我们一般会监听这2个信号，实现在程序进程结构前做一些释放资源的操作
- 编码流程
	注册特定信号的监听, 绑定回调函数
	在监听回调中进行特定处理来响应信号
```

### 应用进程运行模块 

```
包含一个将主应用程序各个模块初始化并运行的函数：app_runner_run
注册终止信号，用来在进程结束前释放资源发
初始化各个模块：线程池、mqtt客户端、设备、蓝牙/串口
启动设备
只要没有收到结束信号，一直不结束，收到结束信号释放资源
```

### main函数

```
如果运行的参数是“app", 调用主应用运行函数启动应用
```

### 编码完善

```
1. 当前时间值开始用的long, 值在板子上运行时为负数， 原因是板子是32位，long是4个字节，存储不了毫秒数 =》 使用long long
2. 串口初始化属性时，需要先初始化其它属性刷新后，再指定为非阻塞模式才有效
3. mqtt的协议从tcp改为ws
```



### 项目交叉编译

```
准备编译烧录工具：
	output.tar.gz  、 sysroot.tar.gz 、 toolchain.tar.gz
	拷贝到项目根目录下，并解压到根目录下
烧录系统
	USB线连接刷机口
	让USB线连接上虚拟机
	刷机
操作开发板的Linux系统
	USB线连接调试口
	让USB线连接上主机(windows)
	利用Xshell通过USB端口连接上开发板Linux系统
	查看开发板Linux系统的IP
	利用Xshell通过IP连接上开发板Linux系统
烧录应用
	编译脚本Makefile, 修改IP为开发板的IP
	执行交叉编译指令：cross-compile
	查看烧录上的程序：ll /usr/bin/gateway
```

### 蓝牙接线测试

```
下位机（32版子）：接上蓝牙和温度传感器
每个组修改下位机程序中的NETID，接电启动
修改模拟终端页面中的IP为电脑的IP, 修改消息主题
上位机（网关版子）：接上蓝牙和网线
修改网关程序中的mqtt的ip和协议/串口文件/蓝牙的NETID和MADDR
重新编译烧录
启动开发版上的程序
```

### OTA模块

```
功能：实现应用的在线自动升级
实现流程：
	1. 请求获取远程固件相关信息：版本号和文件哈希值   json
 	2. 解析json, 得到远程的各级版本号和文件哈希值
	3. 比较远程和本地的版本号，如果远程版本号不大于本地版本号，则无需更新
	4. 如果远程版本号大于本地版本号，则下载远程程序固件
	5. 下载完成后，进行哈希校验，来确认下载的文件没问题
		生成下载完成固件的哈希（使用MD5或SHA加密算法生成），与远程文件哈希比较
	6. 如果不同，校验不通过，删除文件，更新失败
	7. 如果相同，校验通过，重启系统，加载新下载的程序执行
ota_http模块
	请求指定url获取json数据：ota_http_getJson()
	请求指定url下载文件：ota_http_download()
ota_version模块
	实现全流程的版本检查更新：ota_version_checkUpdate()
	每隔一天检查一次：ota_version_checkUpdateDaily()
	输出当前版本号：ota_version_printVersion()
```

### 守护进程模块

```
功能：专门用来守护各个业务功能进程的daemon进程，它不处理任何业务逻辑，业务进程是守护进程的子进程，守护进程主要有2个工作
	1. 启动各个业务子进程（处理业务逻辑）
	2. 在业务进程意外结束后，自动启动业务进程执行（定时检查）

daemon_sub_process模块
	被守护子进程的结构体SubProcess: 
		pid: 进程id
		cmd_param: 命令参数
		fail_count： 失败次数
	进程结构体初始化：daemon_sub_process_init()  不启动子进程
	检查启动子进程：daemon_sub_process_checkStart()  检查子进程是否正在运行，如果没有启动子进程，并在子进程中执行应用程序，并携带指定参数
	结束子进程：daemon_sub_process_stop()

daemon_runner模块:
	daemon进程初始运行：daemon_runner_run()
		将当前进程变为守护进程：daemon()
		关闭标准输入，将标准输出重定向指定的日志文件： close()/open()
		注册进程kill的信号监听，用于守护进程结束前结束被守护的子进程
		初始化所有子进程（app/ota）信息
		每隔2秒检查每个子进程状态，如果没有运行则将其启动起来
		结束前关闭所有子进程
```

### 开机自启动

```
系统启动时，会自动运行/etc/init.d/Sxxx的shell来启动一些程序进程，比如：网络、时针等
那我们也可以模仿已有配置文件的写法来开机自启动我们的程序：S99gateway
开机启动配置的start函数：
	如果OTA程序存在，
		将当前程序移动到备份文件位
		将OTA程序移动到当前程序位置
		给其添加执行权限
	启动当前程序，传入daemon参数
将配置文件传输到开机启动执行的配置文件夹，并给其添加执行权限
reboot重启系统
查看网关进程：有3个，daemon/app/ota, 
杀死app或ota后查看依然存在，杀死daemon,再查看app和ota都不在了
```



### windows下VSCode远程连接Ubuntu虚拟机开发

- windows中，安装vscode远程连接的插件

  ```
  插件名：Remote - SSH
  ```

- 虚拟机中安装并启动 SSH 服务

  ```
  sudo apt install openssh-server
  sudo systemctl start ssh
  sudo systemctl enable ssh
  ```

- windows中测试连接虚拟机

  ```shell
  # 打开powershell中，执行命令:
  ssh 用户名@虚拟机ip
  ```

- windows中配置SSH

  ```shell
  # 导航到C:\Users\<YourUsername>\.ssh\config，添加如下配置（根据实际情况修改）
  Host 虚拟机名
      HostName 虚拟机IP
      User 虚拟机用户名
      Port 22
  ```

- #### windows中, VS Code 连接虚拟机

  ```
  - 打开 VS Code。
  - 按 Ctrl+Shift+P 打开命令面板，输入并选择 "Remote-SSH: Connect to Host..."。
  - 在弹出的列表中选择你刚刚配置的主机名（例如 atguigu）。
  - 输入密码，连接上虚拟机
  ```

  