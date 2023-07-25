#　 dacspreload

## 关于

这个工程主要是做了是实现了两个功能，功能一是剪切板跨进程阻断，功能二为`dns`转发。

## 功能介绍
### 功能一:剪切板跨进程阻断

这个功能最早是通过`hook xlib`的函数进行实现的，先是通过`XGetSelectionOwner`获取所属，接着再调用`XConvertSelection`，最后就是剪切板真正的主人对目标窗口进行发送数据，比较通用的方案是直接在`XConvertSelection`这里进行审计，不过这里审计会遇到一个问题，就是对于静态使用`xlib`库的程序不能有效的进行`hook`，在对`x11`协议进行分析之后，我们采取了解析`x11`协议的方案，对发送数据的请求进行审计，不过在这里审计，会比较受限，因为是在`writev`函数内了，不会再做`writev`｀、`readv`之类的操作了，否则会引起死锁。

这个问题后面真正工程化之后可解决，`hook`模块启动之后新建一个线程，当检测到复制的行为的时候，全部阻断，并将原始请求发送至工作线程，工作线程去权限审核之后，根据审核结果进行对应的操作。



#### 技术实现原理

1. 在`connect`的时候进行判断，判断是否为`x11`套接字，并进行存储
2. 对`writev`的文件句柄进行判断，判断是否为`x11`套接字，是的话进行协议处理
3. 解析`X_ChangeProperty`命令，这里面做了两件事，第一是去发现自己的主窗口，第二是去发现复制行为，是的话根据配置选项进行相应的动作

> 详细的`x11`协议内容可在wiki中查找x11协议分析



### 功能二:`dns`重设

将进程的`dns`请求转发到指定的服务器去，这个有一个假设的前提，就是所有的`dns`请求都是通过`53`端口进行发送。

这个的作用就是在不同域启动的进程在启动时指定环境变量，指之能够访问对应的`dns`服务器进行正常的解析。

####技术原理实现

1. 获取`IPV4DNSHOST`、`IPV6DNSHOST`、`DNSPORT`这三个环境变量

2. `hook connect、sendto、sendmsg`函数，当发现目标端口为`53`端口时，根据协议，进行相应的替换，比如`ipv4`协议的话，使用`IPV4DNSHOST`环境变量的值进行替换，如值未指定，即不进行替换。



## 编译
拉取代码之后，在工程平级目录下新建`build`目录，整个目录结构如下：

```
datacloak@datacloak-PC:~$ tree DACS_Preload build
DACS_Preload
├── bak.ld.so.preload
├── CheckAtomName.c
├── CMakeLists.txt
├── compile_flags.txt
├── connection.c
├── dacspreload.c
├── dacs_wayland-client-core.h
├── ld.so.preload
├── loger.c
├── loger.h
├── Makefile
├── message.c
├── message.h
├── Monitor.c
├── printselect.c
├── readme.md
├── sockets.c
├── sockets.h
├── Start
├── Stats
├── Stop
├── wayland-client.c
├── wayland-client.h
├── wayland-os.c
├── wayland-os.h
├── wayland-private.h
├── window_manage.c
└── window_manage.h
build
```

接着进入到`build`目录中去，执行如下命令：

```
cmake ../DACS_Preload
make
```

示例如下：

```
datacloak@datacloak-PC:~/build$ cmake ../DACS_Preload/
-- The C compiler identification is GNU 8.3.0
-- The CXX compiler identification is GNU 8.3.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- CMAKE_CXX_FLAGS = -fPIC -shared -pthread -lpthread -ldl
RELEASE_BUILD is OFF
CMAKE_CXX_FLAGS is -fPIC -shared -pthread -lpthread -ldl -g
CMAKE_C_FLAGS is -fPIC -shared -pthread -lpthread -ldl -g
HOOK_SOCKET is OFF
HOOK_BIND is OFF
HOOK_CONNECT is ON
HOOK_SENDMSG is ON
HOOK_RECV is OFF
-- Configuring done
-- Generating done
-- Build files have been written to: /home/datacloak/build
datacloak@datacloak-PC:~/build$ make
Scanning dependencies of target dacspreload
[  8%] Building C object CMakeFiles/dacspreload.dir/dacspreload.c.o
[ 16%] Building C object CMakeFiles/dacspreload.dir/sockets.c.o
[ 25%] Building C object CMakeFiles/dacspreload.dir/message.c.o
[ 33%] Building C object CMakeFiles/dacspreload.dir/window_manage.c.o
[ 41%] Linking C shared library libdacspreload.so
[ 41%] Built target dacspreload
Scanning dependencies of target CheckAtomName
[ 50%] Building C object CMakeFiles/CheckAtomName.dir/CheckAtomName.c.o
[ 58%] Linking C executable CheckAtomName
[ 58%] Built target CheckAtomName
Scanning dependencies of target Monitor
[ 66%] Building C object CMakeFiles/Monitor.dir/Monitor.c.o
/home/datacloak/DACS_Preload/Monitor.c: In function ‘thread_func’:
/home/datacloak/DACS_Preload/Monitor.c:110:16: warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]
     clientfd = (int)arg;
                ^
/home/datacloak/DACS_Preload/Monitor.c: In function ‘main’:
/home/datacloak/DACS_Preload/Monitor.c:180:56: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
             ret = pthread_create(&pth,NULL,thread_func,(void*)clientfd);
                                                        ^
[ 75%] Building C object CMakeFiles/Monitor.dir/sockets.c.o
[ 83%] Building C object CMakeFiles/Monitor.dir/loger.c.o
[ 91%] Building C object CMakeFiles/Monitor.dir/message.c.o
[100%] Linking C executable Monitor
[100%] Built target Monitor
datacloak@datacloak-PC:~/build$ ll
总用量 160
-rwxr-xr-x 1 datacloak datacloak 19152 12月 27 18:24 CheckAtomName
-rw-r--r-- 1 datacloak datacloak 13646 12月 27 18:24 CMakeCache.txt
drwxr-xr-x 7 datacloak datacloak  4096 12月 27 18:24 CMakeFiles
-rw-r--r-- 1 datacloak datacloak  1495 12月 27 18:24 cmake_install.cmake
-rwxr-xr-x 1 datacloak datacloak 65896 12月 27 18:24 libdacspreload.so
-rw-r--r-- 1 datacloak datacloak 10520 12月 27 18:24 Makefile
-rwxr-xr-x 1 datacloak datacloak 34656 12月 27 18:24 Monitor
datacloak@datacloak-PC:~/build$
```

编译之后，成功生成`libdacspreload.so`文件。



## 编译开关

可通过指定`cmake`参数来设置编译开关，开关如下：

```
RELEASE_BUILD: 指定是否编译成release版本，即开启优化，去除调试信息的版本，现默认关闭
HOOK_SOCKET: 指定是否对socket函数进行hook，现默认关闭
HOOK_WRITEV:  指定是否对writev函数进行hook，现默认关闭,通过他可控制是否开启拦截剪切板功能
HOOK_BIND: 指定是否对bind函数进行hook，现默认关闭
HOOK_CONNECT: 指定是否对connect函数进行hook，现默认开启
HOOK_SENDMSG: 指定是否对sendmsg函数进行hook，现默认开启
HOOK_RECV: 指定是否对recv函数进行hook，现默认关闭
```

以上的开关状态在构建工程文件的时候，会有对应的日志输出，见上面的示例，因`writev`比较关键，所以这里没有加上编译开关。

如关闭`connect hook`,会影响对`x11`协议的解析。



## 使用

可通过设置`LD_PRELOAD`或者`/etc/ld.so.preload`来让进程加载我们的`so`，使之剪切板生效。

在域内新建进程时，可指定`IPV4DNSHOST`、`IPV6DNSHOST`、`DNSPORT`这三个环境变量的值，使之`DNS`重设功能生效。

示例:

```bash
datacloak@ubuntu:~/build$ export LD_PRELOAD=$(pwd)/libdacspreload.so
datacloak@ubuntu:~/build$ export IPV4DNSHOST=9.9.9.9
datacloak@ubuntu:~/build$ export DNSPORT=99
datacloak@ubuntu:~/build$ echo $LD_PRELOAD
/home/datacloak/build/libdacspreload.so
datacloak@ubuntu:~/build$ echo $IPV4DNSHOST
9.9.9.9
datacloak@ubuntu:~/build$ echo $DNSPORT
99
```

