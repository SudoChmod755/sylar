#include "sylar/config.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include <yaml-cpp/yaml.h>

class Person{
    public:
        std::string m_name="";
        int m_age=0;
        bool m_sex=0;
        std:: string ToString() const {
            std::stringstream ss;
            ss<< "[Person name="<< m_name<<" age="<<m_age<<" sex="<<m_sex<<"]";
            return ss.str();
        }
        bool operator ==(const Person& ohs) const{
            return m_name==ohs.m_name &&
            m_age==ohs.m_age && 
            m_sex==ohs.m_sex;
        }
};

sylar::ConfigVar<Person>:: ptr p_person=sylar::Config::Lookup("class.person.k",Person(),"system_person");

sylar::ConfigVar<int>:: ptr p_sys_port=sylar::Config::Lookup("system.port",(int)8080,"system_port");

sylar::ConfigVar<std::vector<int> >:: ptr g_int_vector_config=sylar::Config::Lookup("system.int_vec",std::vector<int>{1,2},"system int vec");

sylar::ConfigVar<std::list<int> >:: ptr g_int_list_config=sylar::Config:: Lookup("system.int_list",std::list<int>{1,2},"system int list");

sylar::ConfigVar<std::set<int> >:: ptr g_int_set_config=sylar::Config:: Lookup("system.int_set",std::set<int>{1,2},"system int set");

sylar::ConfigVar<std::unordered_set<int> >:: ptr g_int_unset_config=sylar::Config:: Lookup("system.int_unset",std::unordered_set<int>{1,2},"system int unset");

sylar::ConfigVar<std::unordered_map<std::string,int> >:: ptr g_int_unmap_config=sylar::Config:: Lookup("system.int_unmap",std::unordered_map<std::string,int>{{"v",1},{"v2",2}},"system int unmap");

void print_yaml(const YAML::Node& node,int level){
    if(node.IsScalar()){     // 2
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std:: string(level*4,' ')<<node.Scalar()<<
        " - "<<node.Type()<<" - "<<level;
    }
    else if(node.IsNull()){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<std:: string(level*4,' ')<<"NULL - "<<
        node.Type()<<" - "<<level;
    }
    else if(node.IsMap()){         // 4  注意这里是second的type
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
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<" before " << p_sys_port->Tostring();
#define XX(g_var,prefix,var) \
    auto  var=g_var->getVal(); \
    for(auto& i : var){ \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<#prefix<<i;  \
    }

#define XX1(g_var,prefix,var) \
    auto var=g_var->getVal(); \
    for(auto& i: var){ \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<#prefix<<i.first<<" : "<<i.second; \
    }

    XX(g_int_vector_config,before,test1)
    XX(g_int_list_config,before,test2)
    XX(g_int_set_config,before,test5)
    XX(g_int_unset_config,before,test7)
    XX1(g_int_unmap_config,before,test9)


    sylar::Config:: LoadFromYaml(node);

    // auto  v2=g_int_vector_config->getVal();
    // for(auto& i : v2){
    //     SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"after int_vec "<<i;
    // }
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<< " after "<< p_sys_port->Tostring();

    XX(g_int_vector_config,after,test3)
    XX(g_int_list_config,after,test4)
    XX(g_int_set_config,after,test6)
    XX(g_int_unset_config,after,test8)
    XX1(g_int_unmap_config,after,test10)
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<g_int_unset_config->Tostring();


#undef XX
}
namespace sylar{
    template<>
    class LexicalCast<std:: string,Person>{
        public:
            Person operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                Person p;
                p.m_name=node["name"].as<std:: string>();
                p.m_age=node["age"].as<int>();
                p.m_sex=node["sex"].as<bool>();
                return p;
            }
                        

    };

    template<>
    class LexicalCast<Person,std:: string>{
        public:
            std:: string operator() (const Person p){
                YAML::Node node ;
                node["name"]=p.m_name;
                node["age"] =p.m_age;
                node["sex"] =p.m_sex;
                std::stringstream ss;  
                ss<<node;
                return ss.str();          //都是以node（yaml形式的字符串为中转）
            }

    };
  
};
void test_class(const YAML::Node& node){
    p_person->addListener([](const Person& old_person,const Person& new_person){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"this is listener";
    });
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"before "<< p_person->getVal().ToString()<<" - "<< p_person->Tostring();
    sylar::Config:: LoadFromYaml(node);
    SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"after "<< p_person->getVal().ToString()<<" - "<< p_person->Tostring();

}


int main(int argc,char** argv){
    std::cout<<"hello config "<<std::endl;
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << p_sys_port->Tostring();
    YAML::Node root=YAML:: LoadFile("/home/sz123/workspace/sylar/bin/conf/log.yaml");
    
    // print_yaml(root,0);
    //test_config(root);
    test_class(root);
    
    sylar::Config::Visit([](sylar::ConfigVarBase::ptr var){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"name="<<var->getName()<<" description="<<var->getDescrip()<<
         " value="<<var->Tostring();
    });


}