#include <iostream>
#include <string>
#include "test.pb.h"
using namespace fixbug;

int main(){
    // LoginResponse rsp;
    // ResultCode *rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败");

    GetFriendListsResponce rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    User *user1 = rsp.add_friend_list();
    user1->set_name("Zhang san");
    user1->set_age(18);
    user1->set_se(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("Li si");
    user2->set_age(19);
    user2->set_se(User::WOMAN);

    // 获取数据个数
    std::cout<<rsp.friend_list_size()<<std::endl;
    for(const auto& item:rsp.friend_list()){
        std::cout<<item.name()<<" "<<item.age()<<" "<<item.se()<<std::endl;
    }
   // std::cout<<rsp.friend_list(1)<<std::endl;
    return 0;
}


// int main0(){
//     // 封装了login请求对象的数据
//     LoginRequest req;
//     req.set_name("Zhang san");
//     req.set_pwd("123456");

//     std::string send_str;
//     // 将 LoginRequest 对象序列化成一个字符串
//     if(req.SerializeToString(&send_str)){
//         std::cout<<send_str<<std::endl;
//     }

//     // 从send_str反序列化一个login请求对象
//     LoginRequest reqB;
//     if(reqB.ParseFromString(send_str)){
//         std::cout<<reqB.name()<<std::endl;
//         std::cout<<reqB.pwd()<<std::endl;
//     }
//     return 0;
// }