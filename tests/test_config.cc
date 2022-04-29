#include "sylar/config.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include <yaml-cpp/yaml.h>
sylar::ConfigVar<int>:: ptr p_sys_port=sylar::Config::Lookup("system.port",(int)8080,"system_port");

void print_yaml(const YAML::Node& node,int level){
    if(node.IsScalar()){     // 2
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std:: string(level*4,' ')<<node.Scalar()<<
        " - "<<node.Type()<<" - "<<level;
    }
    else if(node.IsNull()){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std:: string(level*4,' ')<<"NULL - "<<
        node.Type()<<" - "<<level;
    }
    else if(node.IsMap()){         // 4 注意这里是second的type
        for(auto i=node.begin();i!=node.end();i++){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std:: string(level*4,' ')<<i->first<<
            " - "<<i->second.Type()<<" - "<< level;
            print_yaml(i->second,level+1);
        }
    }
    else if(node.IsSequence()){          // 3 这里是里面元素的type
        for(size_t i=0;i<node.size();i++){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std:: string(level*4,' ')<<i<<
            " - "<<node[i].Type()<<" - "<< level;
            print_yaml(node[i],level+1);
        }
    }

}

void test_config(const YAML::Node& node){
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<" before " << p_sys_port->Tostring();
    sylar::Config:: LoadFromYaml(node);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<< " after "<< p_sys_port->Tostring();
}

int main(int argc,char** argv){
    std::cout<<"hello config "<<std::endl;
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << p_sys_port->Tostring();
    YAML::Node root=YAML:: LoadFile("/home/sz123/workspace/sylar/bin/conf/log.yaml");
    // print_yaml(root,0);
    test_config(root);

}