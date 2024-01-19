#pragma once

#include "google/protobuf/service.h"
#include <functional>
#include <google/protobuf/descriptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <string>
#include <unordered_map>

//框架提供的专门发布rpc服务的网络对象类
//同时也执行服务端的桩（stub）所负责的一部分工作
class RpcProvider
{
public:
    //这个函数是框架提供给外部发布rpc方法的接口
    //框架是可以接收各种RPC服务的，不能依赖具体的某一个业务
    //基类service指针指向servie的子类对象
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    //muduo的事件循环EventLoop类的对象
    muduo::net::EventLoop m_eventloop;



    //muduo库TCP连接建立时执行的回调函数
    void OnConnection(const muduo::net::TcpConnectionPtr &);

    //muduo库TCP连接上有消息到来（消息读写）时执行的回调函数，相当于服务端的桩stub
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);

   
};
