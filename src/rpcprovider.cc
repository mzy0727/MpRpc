#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
// #include "spdlog/spdlog.h"
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
    LOG_INFO("service_name:{:s}",service_name);
   // std::cout<<"service_name:"<<service_name<<std::endl;
    // 获取服务对象指定下标的服务方法描述（抽象描述）
    for(int i = 0; i < methodCnt; ++i){
        const google::protobuf::MethodDescriptor* pmethdDesc = pserviceDesc->method(i);
        std::string method_name = pmethdDesc->name();
        service_info.m_methodMap.insert({method_name,pmethdDesc});

      //  std::cout<<"method_name:"<<method_name<<std::endl;
      // spdlog::info("method_name: {:s}",method_name.c_str());
        //  LOG_INFO("method_name: {:s}",method_name.c_str());
        // LOG_INFO("method_name: %s",method_name.c_str());
        // LOG_INFO("method_name: %s",method_name.c_str());
        // LOG_ERROR("method_name: %s",method_name.c_str());
        // LOG_DEBUG("method_name: %s",method_name.c_str());
        // LOG_DEBUG("method_name: %s",method_name.c_str());
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

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点  method_name为临时性节点
    for(auto &sp : m_serviceMap){
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp : sp.second.m_methodMap){
            // service_name/method_name     /UserServiceRpc/Login
            // 存储当前这个rpc服务节点的主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            // ZOO_EPHEMERAL表示znode是一个临时节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
            
        }
    }

    //rpc服务端准备启动，打印信息
   // std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
//    spdlog::info("RpcProvider start service at ip:{:s}, port:{:d}",ip.c_str(),port);
    //启动网络服务
    server.start();
    //相当于启动了epoll_wait，阻塞，等待远程连接
    m_eventloop.loop();
}

// 新的socket连接回调,rpc的请求是短连接的，请求一次完了，服务端返回rpc的方法的响应，之后主动关闭连接。
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
   if(!conn->connected()){
        // 和 rpc client的连接断开了
        conn->shutdown();
   }
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

    // 网络上接收的远程rpc调用请求的字符流  Login args
    std::string recv_buf = buffer->retrieveAllAsString();
    std::cout<<"recv_buf:"<<recv_buf<<std::endl;
    //LOG_INFO("recv_buf: {:s}",recv_buf);
    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size,4,0);
    
    // 根据head_size读取数据头的原始字符流,反序列化数据，得到rpc请求的详细消息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    LOG_INFO("rpc_header_str size: {:d}",rpc_header_str.size());
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } else{
         // 数据头反序列化失败
         std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse_error"<<std::endl;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size,args_size);


    // 打印调试信息
    std::cout<< "==================================================="<<std::endl;
    std::cout<< "header_size: "<<header_size << std::endl;
    std::cout<< "rpc_header_str:"<<rpc_header_str<<std::endl;
    std::cout<< "args_str:"<<args_str<<std::endl;
    std::cout<< "==================================================="<<std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        std::cout<< service_name <<" is not exist!" <<std::endl;
        return ;
    }

    
    
    auto method_it = it->second.m_methodMap.find(method_name);
    if(method_it == it->second.m_methodMap.end()){
        std::cout<< service_name<<" : "<< method_name <<" is not exist!" <<std::endl;
        return ;
    }
    // 获取service对象 new UserService
    google::protobuf::Service *service = it->second.m_service;  
    // 获取method对象 
    const google::protobuf::MethodDescriptor *method = method_it->second;

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    // 序列化
    if(!request->ParseFromString(args_str)){
        std::cout<< "request parse error, content: "<<args_str<<std::endl;
        return ;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done =  google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>
                                                                    (this,&RpcProvider::SendRpcResponse,conn,response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // 相当于new UserService().Login(controller,request,response,done)
    service->CallMethod(method,nullptr,request,response,done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response){
    std::string response_str;
    // response进行序列化
    if(response->SerializeToString(&response_str)){
        // 序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);
       // conn->shutdown();   // 模拟http的短链接服务，由rpcprovider主动断开连接    
    }else{
        std::cout<< "Serialize response_str error!"<<std::endl;
    }
    conn->shutdown();   // 模拟http的短链接服务，由rpcprovider主动断开连接 
}