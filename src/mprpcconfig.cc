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
        std::string read_buf(buf);
        Trim(read_buf);
        // int idx = read_buf.find_first_not_of(' ');
        // if(idx != -1){
        //     // 说明字符串前面有空格
        //     read_buf = read_buf.substr(idx,read_buf.size() - idx);
        // }
        // // 去掉字符串后面多余的空格
        // idx = read_buf.find_last_not_of(' ');
        // if(idx != -1){
        //     // 说明字符串后面有空格
        //     read_buf = read_buf.substr(0,idx + 1);
        // }

        // 判断#的注释
        if(read_buf[0] == '#' || read_buf.empty()){
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');
        if(idx == -1){
            // 配置项不合法
            continue;
        }
        std::string key;
        std::string value;
        key = read_buf.substr(0,idx);
        Trim(key);
        // 从=之后找\n
        int endidx = read_buf.find('\n',idx);
        value = read_buf.substr(idx+1,endidx - idx - 1);
        Trim(value);
        m_configMap.insert({key,value});

    }
}
// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf){
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