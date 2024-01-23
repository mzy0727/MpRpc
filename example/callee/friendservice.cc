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
