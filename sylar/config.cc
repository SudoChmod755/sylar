#include "config.h"
#include<sstream>
namespace sylar {
    ConfigVarBase:: ptr Config::LookupConfBase(const std:: string& str){
        auto it=Getdatas().find(str);
        if(it==Getdatas().end()){
            return nullptr;
        }
        return it->second;
    }
    static void ListAllMember(const std::string& prefix,const YAML::Node& node,
    std::list<std:: pair<std::string,YAML::Node > >& node_list){
        if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"invalid key"<<" - "<<prefix<<" - "<<node;
            return ;
        }
        node_list.push_back(std::make_pair(prefix,node));
        if(node.IsMap()){
            for(auto it=node.begin();it!=node.end();++it){
                ListAllMember(prefix.empty()? it->first.Scalar() : prefix+"."+it->first.Scalar() ,
                it->second,node_list);
            }
        }
    }

    void Config:: LoadFromYaml(const YAML::Node& root){
        std::list<std:: pair<std::string,YAML::Node > > node_list;
        ListAllMember("",root,node_list);

        for(auto& it: node_list){
            std:: string& key=it.first;
            if(key.empty()){
                continue;
            }
            std:: transform(key.begin(),key.end(),key.begin(),:: tolower);
            ConfigVarBase:: ptr conpt=LookupConfBase(key);
            if(conpt){
                if(it.second.IsScalar()){
                    conpt->fromstring(it.second.Scalar());
                }
                else {
                    std::stringstream ss;
                    ss<< it.second;
                    conpt->fromstring(ss.str());
                }
            }
        }
    }

    
}