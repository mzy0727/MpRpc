#pragma once

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
/*
header_size + service_name method_name args_size + args
*/
class MprpcChannel : public google::protobuf::RpcChannel{
public:
    // 所有通过stub代理调用的rpc方法，都走到这里了，统一做rpc方法调用端数据序列化和网络发送
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);
    
};