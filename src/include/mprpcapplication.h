#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
// rpc框架的基础类，负责框架的一些初始化操作
class MprpcApplication{
public:
    
    static MprpcApplication* GetInstance(){
        static MprpcApplication application;
        return &application;
    }
    static void Init(int argc, char **argv);
    MprpcConfig GetConfig();
private:
    MprpcApplication(){};
    // 防止拷贝构造和赋值操作，确保单例的唯一性
    MprpcApplication(const MprpcApplication&) =delete;  // 删除拷贝构造函数
    MprpcApplication& operator=(const MprpcApplication&) =delete;   // 拷贝赋值运算符
    MprpcApplication(MprpcApplication&&) =delete;   // 删除移动构造函数

    
private:
    static MprpcConfig m_config;
};