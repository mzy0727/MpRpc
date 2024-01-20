#include "rpcprovider.h"
#include "mprpcapplication.h"


/*
service_name => service描述
                        =》service* 记录服务对象
                        method_name => method方法对象
*/
//这个函数是框架提供给外部发布rpc方法的接口
void RpcProvider::NotifyService(google::protobuf::Service *service){

    ServiceInfo service_info;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count();

    std::cout<<"service_name:"<<service_name<<std::endl;
    // 获取服务对象指定下标的服务方法描述（抽象描述）
    for(int i = 0; i < methodCnt; ++i){
        const google::protobuf::MethodDescriptor* pmethdDesc = pserviceDesc->method(i);
        std::string method_name = pmethdDesc->name();
        service_info.m_methodMap.insert({method_name,pmethdDesc});

        std::cout<<"method_name:"<<method_name<<std::endl;
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info});

}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run(){

    //读取配置文件，获取rpcserver的ip地址和端口号
    std::string ip = MprpcApplication::GetInstance()->GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance()->GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");

    //为新建的服务器绑定连接回调和消息回调函数
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //设置muduo库的线程数量为4，1个是I/O线程，3个是工作线程
    server.setThreadNum(4);

    //rpc服务端准备启动，打印信息
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    //启动网络服务
    server.start();
    //相当于启动了epoll_wait，阻塞，等待远程连接
    m_eventloop.loop();
}

// 新的socket连接回调,rpc的请求是短连接的，请求一次完了，服务端返回rpc的方法的响应，之后主动关闭连接。
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
   
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
怎么商量呢？ 
包含：service_name  method_name   args   
对应：16UserService   Login    zhang san123456   
我们在框架中定义proto的message类型，进行数据头的序列化和反序列化
service_name method_name args_size(防止粘包的问题) 

怎么去区分哪个是service_name, method_name, args
我们用消息头表示 service_name, method_name以及args_size（参数字符串的长度）。

消息的格式是：
header_size + service_name method_name args_size + args
即：
header_size(4个字节) + header_str + args_str
header_str 是service_name，method_name以及args_size。 
为了防止粘包，我们还要用args_str记录表示参数的字符串。 
我们统一：一开始读4个字节，数据头的长度，也就是服务名字，方法名字 和参数字符串的长度；之后再根据参数的字符串的长度，读取参数字符串
10 "10"
10000 "1000000"
std::string   insert和copy方法 
*/
//已建立连接用户的读写事件回调,如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
//主要作用：首先反序列化MprpcChannel::CallMethod()中发过来的消息，之后会调用相应的本地方法，之后序列化响应，并将序列化后的响应返回给MprpcChannel::CallMethod()。
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp){
                                
    }

