#include "mprpccontroller.h"

MprpcController::MprpcController(){
    m_failed = false;
    m_errorText = "";
}
void MprpcController::Reset(){
    m_failed = false;
    m_errorText = "";
}
bool MprpcController::Failed()   const{
    return m_failed;
}
std::string MprpcController::ErrorText() const{
    return m_errorText;
}
void MprpcController::SetFailed(const std::string& reason){
    m_failed = true;
    m_errorText = reason;
}

// 目前未实现
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const{}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback){}