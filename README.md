# 从零实现分布式网络通信框架
# 项目介绍

该项目是在 Linux 环境下基于 muduo、Protobuf 和 Zookeeper 实现的一个轻量级 RPC 框架。可以把单体架构系统的本地方法调用，重构成基于 TCP 网络通信的 RPC 远程方法调用，实现统一台机器不同进程或者不同机器之间的服务调用。

# 项目特点

- 基于 muduo 网络库实现高并发网络通信模块，作为 RPC 远程调用的基础。
- 基于 Protobuf 实现 RPC 方法调用和参数的序列化和反序列化，并根据其提供得 RPC 接口编写 RPC 服务。
- 基于 ZooKeeper 分布式协调服务中间件提供服务注册和服务发现功能。
- 基于生产者消费者模型，实现了异步工作方式的日志模块。

# 项目演示

项目构建，运行自动编译脚本，生成可执行程序。

```bash
chmod +x autobuild.sh #给自动编译脚本添加可执行权限
./autobuild.sh #启动自动编译脚本

# 执行完后，会在当前目录的bin目录下生成可执行文件
[mprpc]$ tree ./bin/
./bin/
|-- consumer
|-- mprpc.conf
`-- provider

0 directories, 3 files

# mprpc.conf 是 rpc 节点和 Zookeeper 的配置信息
[mprpc]$ cat ./bin/mprpc.conf 
# rpc节点的ip地址
rpcserverip=127.0.0.1
# rpc节点的端口号
rpcserverport=8080
# zookeeper的ip地址
zookeeperip=127.0.0.1
# zookeeper的端口号
```

启动 Zookeeper，我们需要再 Zookeeper 上获取注册的服务消息，因此先需要保证启动了 Zookeeper。进入到 Zookeeper 的目录，我的 Zookeeper 目录是在`/home/Joy/install/zookeeper-3.4.10`。在里面的`bin`目录下有客户端和服务端的启动脚本，先启动 Zookeeper 服务端，再启动客户端。启动客户端主要是为了看我们的 RPC 框架是否插入新服务信息。

```bash
./zkServer.sh start #启动 Zookeeper 服务端
./zkCli.sh #启动 Zookeeper 客户端
```

![1](https://gitee.com/DreamyHorizon/mprpc/raw/master/image/1_20230821225819.png)

启动服务端

```bash
./provider -i mprpc.conf
```

我们观察下打印的信息，可以看到它打印了 `mprpc.conf` 配置文件的信息，至少说明 RPC 框架读取配置是成功的。

![1](https://gitee.com/DreamyHorizon/mprpc/raw/master/image/2_20230821231918.png)

重点是下面的信息，其显示在 Zookeeper 上注册了服务。

![3_20230821232424](https://gitee.com/DreamyHorizon/mprpc/raw/master/image/3_20230821232424.png)

启动客户端

```bash
./consumer -i mprpc.conf
```

![4_20230821233410](https://gitee.com/DreamyHorizon/mprpc/raw/master/image/4_20230821233410.png)

有很多的提示信息，也是解析了配置文件，并且有许多 Zookeeper 相关日志信息。这里注意最重要的几个地方，它打印显示了 `rpc Login response : 1`。响应为 1，RPC 方法调用成功！

## 集群与分布式

集群：每一台服务器独立运行一个工程的所有模块

分布式：一个工程拆分成很多模块，每一个模块独立部署运行在一个服务器主机上，所有服务器协同工作共同提供服务，每一台服务器称作分布式的一个节点，根据节点的并发要求，对一个节点可以再做节点模块集群部署。

### 单机服务器

![单机服务器](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/%E5%8D%95%E6%9C%BA.svg)

缺点： 

1. 受限于硬件资源，聊天服务器所能承受的用户的并发量
2. 任意模块的更改，都会导致整个项目代码重新编译、部署
3. 系统中，有些模块是属于CPU密集型的，有些模块是属于`I/O`密集型的，造成各模块对于硬件资源的需求是不一样的

### 集群

![集群服务器](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/%E9%9B%86%E7%BE%A4%E6%9C%8D%E5%8A%A1%E5%99%A8.svg)

优点：并发量提升了

缺点：项目代码还是需要整体重新编译，而且需要进行多次部署后台管理这样的模块，根本不需要高并发

### 分布式

所有服务器共同构成一个系统，把系统拆分成模块 放到每台服务器上，需要高并发的模块就多放几台，容易挂的也多放几台，资源需求就蛮少

![分布式](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/%E5%88%86%E5%B8%83%E5%BC%8F.svg)

优点：解决单机的三个缺点，集群的缺点

缺点：系统复杂度大大增加，对基础架构要求更高

1. **大系统的软件模块该怎么划分**？个个模块可能会实现大量重复，需要架构的比较好
2. 各模块之间该怎么访问呢？**机器1上的一个模块进程1怎么调用机器2是的模块进程2里面的业务方法呢**？各个模块都运行在不同的进程里，docker虚拟化环境中

![image-20240111211235235](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/image-20240111211235235.png)

## RPC（远程过程调用）

### RPC通信原理

![通信原理](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/%E5%88%86%E5%B8%83%E5%BC%8F%E6%A1%86%E6%9E%B6.svg)

`User-stub`部分：设计`rpc`方法参数的打包和解析，也就是数据的序列化和反序列化，使用`Protobuf`。 

`RPCRuntime`部分：网络部分，包括寻找`rpc`服务主机，发起`rpc`调用请求和响应`rpc`调用结果，使用`muduo`网络库和`zookeeper`服务配置中心（专门做服务发现）。

`protobuf`相比`json`的好处:

1. `protobuf`是二进制存储，`xml`和`json`都是文本存储的
2. `protobuf`不需要存储额外的信息；`json`需要存储键

![img](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/2020091816070210.png)

**序列化就是将对象转换成二进制数据的过程，而反序列就是反过来将二进制转换为对象的过程**。**网络传输的数据必须是二进制数据，所以在 RPC 调用中，对入参对象与返回值对象进行序列化与反序列化是一个必须的过程。**

![img](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/20200918160833181.png)

## 如何把本地服务通过`proto`发布成远程服务

**本地服务**

```c++
#include <iostream>
#include <string>

/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/

class UserService{
public:
    bool Login(std::string name, std::string pwd){
        std::cout << "doing local service Login"<<std::endl;
        std::cout<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
    }
};

int main(){
    UserService us;
    us.Login("xxx","xxx");

    return 0;
}

```

新建一个`proto`文件

```protobuf
syntax = "proto3";

package fixbug;

option cc_generic_services = true;

message ResultCode{
    int32 errcode = 1;
    bytes errmsg = 2;
}

message LoginRequest{
    bytes name = 1;
    bytes pwd = 2;
}

message LoginResponse{
    ResultCode result = 1;
    bool sucess = 2;
}

service UserServiceRpc{
    rpc Login(LoginRequest) returns(LoginResponse);
}
```

使用`protoc user.proto --cpp_out=./`，编译`user.proto`文件生成`.cpp`文件

```c++
#include <iostream>
#include <string>
#include "user.pb.h"

using namespace fixbug;
/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/

class UserService:public UserServiceRpc{    // 使用在rpc服务的发布端（rpc服务提供者）
public:
    bool Login(std::string name, std::string pwd){
        std::cout << "doing local service Login"<<std::endl;
        std::cout<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
    }
    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller ==> Login(LoginRequest) ==> moduo ==> callee
    2. callee ==> Login(LoginRequest) ==> 交到下面重写的这个Login方法上了
    */ 
    void Login(::google::protobuf::RpcController* controller,
                    const ::fixbug::LoginRequest* request,
                    ::fixbug::LoginResponse* response,
                    ::google::protobuf::Closure* done){
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name,pwd);

        // 把响应写入 包括错误码、错误信息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作  
        // 执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }

};

int main(){
    UserService us;
    us.Login("xxx","xxx");

    return 0;
}
```

{% note success %}

问题：已经在文件中`#include "user.pb.h"`，但是`class UserService:public UserServiceRpc`中的`UserServiceRpc`还是报错，显示找不到这个类

解决方法：`using namespace fixbug;`或者

`class UserService:public fixbug::UserServiceRpc`

{% endnote %}

## 设计框架

首先，根据想法，应该怎么设计框架，先调用框架的初始化操作`Init`，然后创建一个`rpc`网络服务对象，发布一个服务`NotifyService`，然后启动`rpc`服务。

```c++
#include <iostream>
#include <string>
#include "user.pb.h"

using namespace fixbug;
/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/
class UserService:public UserServiceRpc{    // 使用在rpc服务的发布端（rpc服务提供者）
public:
    bool Login(std::string name, std::string pwd){
        std::cout << "doing local service Login"<<std::endl;
        std::cout<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
    }
    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller ==> Login(LoginRequest) ==> moduo ==> callee
    2. callee ==> Login(LoginRequest) ==> 交到下面重写的这个Login方法上了
    */ 
    void Login(::google::protobuf::RpcController* controller,
                    const ::fixbug::LoginRequest* request,
                    ::fixbug::LoginResponse* response,
                    ::google::protobuf::Closure* done){
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name,pwd);

        // 把响应写入 包括错误码、错误信息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作  
        // 执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }

};

int main(int argc, char **argv){
    UserService us;
    us.Login("xxx","xxx");

    // 调用框架的初始化操作
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    //provider.NotifyService(new ProductService());

    // 启动一个rpc服务发布节点,Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}
```

然后根据前面的设计，完善类和方法

**mprpcapplication.h**

```c++
#pragma once

// rpc框架的基础类，负责框架的一些初始化操作
class MprpcApplication{
public:
    MprpcApplication(){};
    static MprpcApplication* GetInstance(){
        static MprpcApplication application;
        return &application;
    }
    // 防止拷贝构造和赋值操作，确保单例的唯一性
    MprpcApplication(const MprpcApplication&) =delete;  // 删除拷贝构造函数
    MprpcApplication& operator=(const MprpcApplication&) =delete;   // 拷贝赋值运算符
    MprpcApplication(MprpcApplication&&) =delete;   // 删除移动构造函数

    static void Init(int argc, char **argv);

};
```

**mprpcapplication.cc**

```c++
#include "mprpcapplication.h"

// 类外实现静态方法不用static
void MprpcApplication::Init(int argc, char **argv){

}
```

**rpcprovider.h**

```c++
#pragma once
#include "google/protobuf/service.h"

// 框架提供的专门发布rpc服务的网络对象类
class RpcProvider{
public:
    //RpcProvider(){};
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    
    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

};
```

**rpcprovider.cc**

```c++
#include "rpcprovider.h"

void RpcProvider::NotifyService(google::protobuf::Service *service){

};

void RpcProvider::Run(){

};
```

进入`build`文件夹，运行`cmake ..`编译生成`MakeFile`文件，然后`make`，根据生成`makefile`文件，编译程序。生成包括可执行文件`provider`和动态库`libmprpc.so`。

![image-20240113141045539](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/image-20240113141045539.png)



{% note success %}

**类的静态成员**

特点：
　　A、内存分配：在程序的全局数据区分配。
　　B、初始化和定义：
　　　 a、静态数据成员定义时要分配空间，所以不能在类声明中定义。
　　　 b、为了避免在多个使用该类的源文件中，对其重复定义，所在，不能在类的头文件中
　　　　定义。
　　　 c、静态数据成员因为程序一开始运行就必需存在，所以其初始化的最佳位置在类的内部实现。
　　C、特点
　　　 a、对相于`public,protected,private`关键字的影响它和普通数据成员一样，
　　　 b、因为其空间在全局数据区分配，属于所有本类的对象共享，所以，它不属于特定的类对象，在没产生类对象时其作用域就可见，即在没有产生类的实例时，我们就可以操作它。
　　D、访问形式
　　　 a、 类对象名.静态数据成员名

   E、静态数据成员，主要用在类的所有实例都拥有的属性上。比如，对于一个存款类，帐号相对于每个实例都是不同的，但每个实例的利息是相同的。所以，应该把利息设为存款类的静态数据成员。这有两个好处，第一，不管定义多少个存款类对象，利息数据成员都共享分配在全局区的内存，所以节省存贮空间。第二，一旦利息需要改变时，只要改变一次，则所有存款类对象的利息全改变过来了，因为它们实际上是共用一个东西。

**定义类的静态成员**

一般来说，不能在类的内部初始化静态成员。必须在类的外部定义和初始化每个静态成员。和其他对象一样，一个静态数据成员只能定义一次。静态数据成员定义在任何函数之外。因此一旦它被定义，就将一直存在于程序的整个生命周期中。

**外部定义**

```c++
MprpcConfig MprpcApplication::m_config;
```

**类内初始化**

通常情况下，类的静态成员不应该在类的内部初始化。然而，可以为静态成员提供`const`整数类型的类内初始值，不过要求静态成员必须是字面值常量类型的`constexpr`。

```c++
static const MprpcConfig m_config;
```

{% endnote %}

## 读取配置文件

### **mprpcapplication.h**

```cpp
#pragma once
#include "mprpcconfig.h"
// rpc框架的基础类，负责框架的一些初始化操作
class MprpcApplication{
public:
    MprpcApplication(){};
    static MprpcApplication* GetInstance(){
        static MprpcApplication application;
        return &application;
    }
    static void Init(int argc, char **argv);
private:
    // 防止拷贝构造和赋值操作，确保单例的唯一性
    MprpcApplication(const MprpcApplication&) =delete;  // 删除拷贝构造函数
    MprpcApplication& operator=(const MprpcApplication&) =delete;   // 拷贝赋值运算符
    MprpcApplication(MprpcApplication&&) =delete;   // 删除移动构造函数

    
private:
    static MprpcConfig m_config;
};
```

### **mprpcapplication.cpp**

```c++
#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp(){
    std::cout<<"format: command -i <configfile>" << std::endl;
}

// 类外实现静态方法不用static
void MprpcApplication::Init(int argc, char **argv){
    if(argc < 2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1){
        switch(c){
            case 'i':
                config_file = optarg; 
                break;
            case '?':
                std::cout<<"invalid args!" << std::endl;
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                std::cout<<"need <configfile>" <<std::endl;
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    // 开始加载配置文件
    m_config.LoadConfigFile(config_file.c_str());
    // 测试
    std::cout<<"rpcserverip: "<<m_config.Load("rpcserverip")<<std::endl;
    std::cout<<"rpcserverport: "<<m_config.Load("rpcserverport")<<std::endl;
    std::cout<<"zookeeperip: "<<m_config.Load("zookeeperip")<<std::endl;
    std::cout<<"zookeeperport: "<<m_config.Load("zookeeperport")<<std::endl;
}
```

### mprpcconfig.h

```c++
#pragma once
#include<unordered_map>
#include<string>

// rpcserverip  rpcserverport  zookeeperip   zookeeperport
// 框架读取配置文件类
class MprpcConfig{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char* config_file);
    // 查询配置项信息
    std::string Load(const std::string &key);
private:
    std::unordered_map<std::string, std::string> m_configMap;   // 保存配置项

};
```

### mprpcconfig.cpp

```c++
#include "mprpcconfig.h"
#include <iostream>

void MprpcConfig::LoadConfigFile(const char* config_file){
    FILE *fp = fopen(config_file, "r");
    if(fp == nullptr){
        std::cout<< config_file << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 1.注释  2.正确的配置项 =  3. 去掉前后多余的空格
    while(!feof(fp)){
        char buf[512] ={0};
        fgets(buf,512,fp);

        // 去掉字符串前面多余的空格
        std::string src_buf(buf);
        int idx = src_buf.find_first_not_of(' ');
        if(idx != -1){
            // 说明字符串前面有空格
            src_buf = src_buf.substr(idx,src_buf.size() - idx);
        }
        // 去掉字符串后面多余的空格
        idx = src_buf.find_last_not_of(' ');
        if(idx != -1){
            // 说明字符串后面有空格
            src_buf = src_buf.substr(0,idx + 1);
        }

        // 判断#的注释
        if(src_buf[0] == '#' || src_buf.empty()){
            continue;
        }

        // 解析配置项
        idx = src_buf.find('=');
        if(idx == -1){
            // 配置项不合法
            continue;
        }
        std::string key;
        std::string value;
        key = src_buf.substr(0,idx);
        value = src_buf.substr(idx+1,src_buf.size() - idx);
        m_configMap.insert({key,value});

    }
}
    // 查询配置项信息
std::string MprpcConfig::Load(const std::string &key){
    // 不用用m_configMap[key],因为当key不存在时，会往map中加一项
   // return m_configMap[key];
   auto it = m_configMap.find(key);
   if(it == m_configMap.end()){
        return "";
   }
   return it->second;
}
```

```markdown
doing local service Login
name:xxxpwd:xxx
rpcserverip: 127.0.0.1

rpcserverport: 8000

zookeeperip: 127.0.0.1

zookeeperport: 5000

```

## `muduo`库的安装与使用

下载`muduo`库

```shell
git clone https://github.com/chenshuo/muduo
```

安装`CMake`

```shell
sudo apt-get install cmake
```

安装`boost`库

```shell
sudo apt-get install libboost-dev libboost-test-dev
```

`muduo`有三个非必须的依赖库，`curl、c-ares DNS、google protobuf`，如果安装了这三个库，`cmake`会多编译一些示例：

```shell
sudo apt-get install libcurl4-openssl-dev libc-ares-dev
sudo apt-get install protobuf-compiler libprotobuf-dev
```

安装好前面的依赖后，正式开始安装`muduo`

```shell
#进入muduo目录
cd muduo
#编译
./build.sh
```

编译完成后，在输入`./build.sh install`命令进行`muduo`库安装

```shell
./build.sh install
```

`./build.sh install`实际上把`muduo`的头文件和lib库文件放到了`muduo`同级目录下的`build`目录下的`release-install-cpp11`文件夹下面了：

```shell
mzy@DESKTOP-4QU667E:/media/mzy/build$ ls
release-cpp11  release-install-cpp11
```

`install`命令并没有把它们拷贝到系统路径下，导致每次编译程序都需要指定`muduo`库的头文件和库文件路径，很麻烦，所以选择直接把`inlcude`（头文件）和`lib`（库文件）目录下的文件拷贝到系统目录下

```shell
sudo mv muduo/ /usr/include/
sudo mv * /usr/local/lib/
```

**测试代码**

```cpp
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <muduo/net/EventLoop.h>

// 使用muduo开发回显服务器
class EchoServer
{
 public:
  EchoServer(muduo::net::EventLoop* loop,
             const muduo::net::InetAddress& listenAddr);

  void start(); 

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn);

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp time);

  muduo::net::TcpServer server_;
};

EchoServer::EchoServer(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& listenAddr)
  : server_(loop, listenAddr, "EchoServer")
{
  server_.setConnectionCallback(
      boost::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
  server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp time)
{
  // 接收到所有的消息，然后回显
  muduo::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.toString();
  conn->send(msg);
}


int main()
{
  LOG_INFO << "pid = " << getpid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(8888);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
}

```

使用g++进行编译，注意链接`muduo`和`pthread`的库文件，编译命令如下：

```shell
g++ main.cpp -lmuduo_net -lmuduo_base -lpthread -std=c++11
```

编译链接完成，生成`a.out`可执行程序，上面的echo服务器监听8888端口，运行上面的`a.out`回显服务器如下：

```shell
20240119 07:47:16.570523Z  9085 INFO  pid = 9085 - main.cpp:61
20240119 07:47:34.910773Z  9085 INFO  TcpServer::newConnection [EchoServer] - new connection [EchoServer-0.0.0.0:8888#1] from 127.0.0.1:53666 - TcpServer.cc:80
20240119 07:47:34.910814Z  9085 INFO  EchoServer - 127.0.0.1:53666 -> 127.0.0.1:8888 is UP - main.cpp:42
20240119 07:47:34.910841Z  9085 INFO  EchoServer-0.0.0.0:8888#1 echo 12 bytes, data received at 1705650454.910821 - main.cpp:53
```

等待客户端连接，可以打开一个新的`shell`命令行用`netcat`命令模拟客户端连接echo服务器进行功能测试，命令如下：

```shell
echo "hello world" | nc localhost 8888
hello world
```





```shell
sudo find /usr -name "libmuduo*"
[sudo] password for mzy:
/usr/local/lib/libmuduo_base.a
/usr/local/lib/libmuduo_protorpc.a
/usr/local/lib/libmuduo_net.a
/usr/local/lib/libmuduo_protobuf_codec.a
/usr/local/lib/libmuduo_protorpc_wire.a
/usr/local/lib/libmuduo_http.a
/usr/local/lib/libmuduo_inspect.a
```

```shell
cmake --build build
[ 14%] Linking CXX shared library /media/mzy/learn_mprpc/lib/libmprpc.so
/usr/bin/ld: /usr/local/lib/libmuduo_net.a(EventLoop.cc.o): relocation R_X86_64_TPOFF32 against `_ZN12_GLOBAL__N_118t_loopInThisThreadE' can not be used when making a shared object; recompile with -fPIC
/usr/bin/ld: /usr/local/lib/libmuduo_net.a(InetAddress.cc.o): relocation R_X86_64_TPOFF32 against `_ZL15t_resolveBuffer' can not be used when making a shared object; recompile with -fPIC
collect2: error: ld returned 1 exit status
make[2]: *** [src/CMakeFiles/mprpc.dir/build.make:129: /media/mzy/learn_mprpc/lib/libmprpc.so] Error 1
make[1]: *** [CMakeFiles/Makefile2:131: src/CMakeFiles/mprpc.dir/all] Error 2
make: *** [Makefile:91: all] Error 2
```

`muduo`库是静态库，而自己生成的`mprpc`库是动态库，动态库在链接静态库的时候可能会出现问题，现在有两个解决方法：

1.  把`muduo`重新编译成动态库
2.  把`mprpc`编译成静态库

```cmake
aux_source_directory(. SRC_LIST)
add_library(mprpc STATIC ${SRC_LIST})
target_link_libraries(mprpc muduo_base muduo_net pthread)
```

```c++
#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>

// 框架提供的专门发布rpc服务的网络对象类
class RpcProvider{
public:
    //RpcProvider(){};
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    
    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
   
    // muduo的事件循环EventLoop类的对象
    muduo::net::EventLoop m_eventLoop;
    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    // 已建立连接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer*, muduo::Timestamp);
};
```

```c++
#include "rpcprovider.h"
#include "mprpcapplication.h"
#include <string>

void RpcProvider::NotifyService(google::protobuf::Service *service){

};

void RpcProvider::Run(){
    std::string ip = MprpcApplication::GetInstance()->GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance()->GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    
    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    // 绑定连接回调和消息读写回调方法   分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,
    std::placeholders::_2,std::placeholders::_3));
    
    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
};

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr&){

 }

 // 已建立连接用户的读写事件回调
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp){

}
```

## 发布服务

Callee是`RpcProvider`的使用者，`rpc`服务方法的发布方

```markdown
原
class UserService{
login 本地方法
}
```

||

```markdown
继承RPC方法的类
class UserService : public UserServiceRpc{
login 本地方法
login 重写protobuf提供的virtual虚函数{
	1. 从LoginRequest获取参数的值
	2. 执行本地服务login，并获取返回值
	3. 用上面的返回值填写LoginResponse
	4. 一个回调，把LoginResponse发送到rpc client（rpc服务的调用端）
}

}
```

![RpcProvider框架](https://mzy777.oss-cn-hangzhou.aliyuncs.com/img/RpcProvider%E6%A1%86%E6%9E%B6.svg)



### 用户服务

```c++
#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

using namespace fixbug;
/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
*/
class UserService:public UserServiceRpc{    // 使用在rpc服务的发布端（rpc服务提供者）
public:
    bool Login(std::string name, std::string pwd){
        std::cout << "doing local service Login"<<std::endl;
        std::cout<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
        return true;
    }
    
    bool Register(int id, std::string name, std::string pwd){
        std::cout << "doing local service Register"<<std::endl;
        std::cout<<"id: "<<id<<"name: "<<name<<"pwd: "<<pwd<<std::endl;
        return true;
    }
    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller ==> Login(LoginRequest) ==> moduo ==> callee
    2. callee ==> Login(LoginRequest) ==> 交到下面重写的这个Login方法上了
    */ 
    void Login(::google::protobuf::RpcController* controller,
                    const ::fixbug::LoginRequest* request,
                    ::fixbug::LoginResponse* response,
                    ::google::protobuf::Closure* done){
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name,pwd);

        // 把响应写入 包括错误码、错误信息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作  
        // 执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }
    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done){
        // 框架给业务上报了请求参数RegisterRequest，应用获取相应数据做本地业务
        int id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 本地业务
        bool register_result = Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(register_result);

        done->Run();
    }
};

int main(int argc, char **argv){
    UserService us;
   // us.Login("xxx","xxx");

    // 调用框架的初始化操作 provider -i config.conf
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    //provider.NotifyService(new ProductService());

    // 启动一个rpc服务发布节点,Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}
```

```shell
./provider -i test.conf

rpcserverip: 127.0.0.1
rpcserverport: 8000
zookeeperip: 127.0.0.1
zookeeperport: 5000
service_name:UserServiceRpc
method_name:Login
method_name:Register
RpcProvider start service at ip:127.0.0.1 port:8000
20240123 01:51:28.667152Z  4297 INFO  TcpServer::newConnection [RpcProvider] - new connection [RpcProvider-127.0.0.1:8000#1] from 127.0.0.1:48854 - TcpServer.cc:80
===================================================
header_size: 25
rpc_header_str:
UserServiceRpcLogin
args_str:
        Zhang san123456
===================================================
doing local service Login
name:Zhang sanpwd:123456
20240123 01:51:28.667959Z  4297 INFO  TcpServer::removeConnectionInLoop [RpcProvider] - connection RpcProvider-127.0.0.1:8000#1 - TcpServer.cc:109
20240123 01:51:28.668079Z  4297 INFO  TcpServer::newConnection [RpcProvider] - new connection [RpcProvider-127.0.0.1:8000#2] from 127.0.0.1:48856 - TcpServer.cc:80
===================================================
header_size: 28
rpc_header_str:
UserServiceRpRegister
args_strLi Si⸮abcde
===================================================
doing local service Register
id: 1name: Li Sipwd: abcde
20240123 01:51:28.668910Z  4297 INFO  TcpServer::removeConnectionInLoop [RpcProvider] - connection RpcProvider-127.0.0.1:8000#2 - TcpServer.cc:109
```

```shell
./consumer -i test.conf 
rpcserverip: 127.0.0.1
rpcserverport: 8000
zookeeperip: 127.0.0.1
zookeeperport: 5000
===================================================
header_size: 25
rpc_header_str: 
UserServiceRpcLogin
service_name: UserServiceRpc
method_name: Login
args_str: 
        Zhang san123456
===================================================
rpc login response: 1
===================================================
header_size: 28
rpc_header_str: 
UserServiceRpRegister
service_name: UserServiceRpc
method_name: Register
args_str:Li Siabcde
===================================================
rpc register response: 1
```

### 好友服务

```c++
#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>

/*
FriendService原来是一个本地服务，提供了两个进程内的本地方法，GetFriendLists
*/
class FriendService:public fixbug::FriendServiceRpc{    // 使用在rpc服务的发布端（rpc服务提供者）
public:

    std::vector<std::string> GetFriendsList(uint32_t uid){
        std::cout << "doing local service GetFriendsList,uid = "<<uid<<std::endl;
        std::vector<std::string> friendslist;
        friendslist.emplace_back("Long aotian");
        friendslist.emplace_back("Zhao ritian");
        friendslist.emplace_back("Ye fan");
        friendslist.emplace_back("Xiao yan");
        return friendslist;
    }
    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller ==> Login(LoginRequest) ==> moduo ==> callee
    2. callee ==> Login(LoginRequest) ==> 交到下面重写的这个Login方法上了
    */ 
    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done){
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        uint32_t uid = request->uid();

        // 做本地业务
        std::vector<std::string> firendslist = GetFriendsList(uid);

        // 把响应写入 包括错误码、错误信息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        

        for(auto &name : firendslist){
            std::string* p = response->add_friends();
            *p = name;
        }
        // 执行回调操作  
        // 执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }
    
};

int main(int argc, char **argv){
    FriendService us;
   // us.Login("xxx","xxx");

    // 调用框架的初始化操作 provider -i config.conf
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    //provider.NotifyService(new ProductService());

    // 启动一个rpc服务发布节点,Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}

```

```shell
./provider -i test.conf
rpcserverip: 127.0.0.1
rpcserverport: 8000
zookeeperip: 127.0.0.1
zookeeperport: 5000
service_name:FriendServiceRpc
method_name:GetFriendsList
RpcProvider start service at ip:127.0.0.1 port:8000
20240123 02:36:02.487174Z 10070 INFO  TcpServer::newConnection [RpcProvider] - new connection [RpcProvider-127.0.0.1:8000#1] from 127.0.0.1:48890 - TcpServer.cc:80
===================================================
header_size: 36
rpc_header_str:
FriendServiceRpcGetFriendsList
args_str:
===================================================
doing local service GetFriendsList,uid = 9
20240123 02:36:02.487649Z 10070 INFO  TcpServer::removeConnectionInLoop [RpcProvider] - connection RpcProvider-127.0.0.1:8000#1 - TcpServer.cc:109
```

```shell
./consumer -i test.conf 
rpcserverip: 127.0.0.1
rpcserverport: 8000
zookeeperip: 127.0.0.1
zookeeperport: 5000
===================================================
header_size: 36
rpc_header_str: 
FriendServiceRpcGetFriendsList
service_name: FriendServiceRpc
method_name: GetFriendsList
args_str: 
===================================================
rpc GetFriendsList response: 0
index: 1 name: Long aotian
index: 2 name: Zhao ritian
index: 3 name: Ye fan
index: 4 name: Xiao yan
```

### 日志
```
Total log size: 164068760 bytes
Logging 2000000 entries took 0.893891 seconds
Rate: 175.041574 MB/second

```