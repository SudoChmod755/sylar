#ifndef __SYLAR_CONFIG_H_
#define __SYLAR_CONFIG_H_
#include<memory>
#include<string>
#include<boost/lexical_cast.hpp>
#include<sstream>
#include "log.h"
#include "util.h"
#include<yaml-cpp/yaml.h>
#include<unordered_map>
#include<unordered_set>
#include<set>
#include<list>

namespace sylar{

    class ConfigVarBase{
        public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string& name,const std::string& description=""):m_name(name),m_descrip(description){
            std::transform(m_name.begin(),m_name.end(),m_name.begin(),:: tolower);
        }   
        virtual ~ConfigVarBase(){};
        const std:: string& getName() const {
            return m_name;
        }
        const std:: string& getDescrip(){
            return m_descrip;
        }
        
        virtual std::string Tostring()=0;

        virtual bool fromstring(const std:: string& val)=0;

        virtual std:: string getTypename() const =0;

        protected:
            std:: string m_name;
            std:: string m_descrip;
    };
// ------------------ 主模板 -------------
    template<class F,class T>
    class LexicalCast{
        public:
            T operator() (const F& v){
                return boost:: lexical_cast<T>(v);
            }
    };
// ------------------ 偏特化 -------------

// vector
    template<class T>
    class LexicalCast<std:: string,std:: vector<T> >{
        public:
            std:: vector<T> operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                typename std::vector<T> vec;
                std:: stringstream ss;
                for(size_t i=0;i<node.size();i++){
                    ss.str("");
                    ss<<node[i];
                    vec.push_back(
                         LexicalCast<std::string,T>()(ss.str())
                    );
                }
                return vec;
            }                //利用了yaml的sequence

    };

    template<class T>
    class LexicalCast<std:: vector<T>, std:: string>{
        public:
            std:: string operator() (const std:: vector<T>& v){
                YAML::Node node ;
                for(auto& i :v){
                    node.push_back(YAML:: Load( LexicalCast<T,std:: string>() (i) ) );
                }
                std:: stringstream ss;
                ss<<node;
                return ss.str();               
            }

    };

//  list 
template<class T>
    class LexicalCast<std:: string,std:: list<T> >{
        public:
            std:: list<T> operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                typename std::list<T> lis;
                std:: stringstream ss;
                for(size_t i=0;i<node.size();i++){
                    ss.str("");
                    ss<<node[i];
                    lis.push_back(
                         LexicalCast<std::string,T>()(ss.str())
                    );
                }
                return lis;
            }                

    };

    template<class T>
    class LexicalCast<std:: list<T>, std:: string>{
        public:
            std:: string operator() (const std:: list<T>& v){
                YAML::Node node ;
                for(auto& i :v){
                    node.push_back(YAML:: Load( LexicalCast<T,std:: string>() (i) ) );
                }
                std:: stringstream ss;
                ss<<node;
                return ss.str();               
            }

    };
 
//  set
template<class T>
    class LexicalCast<std:: string,std:: set<T> >{
        public:
            std:: set<T> operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                typename std::set<T> see;
                std:: stringstream ss;
                for(size_t i=0;i<node.size();i++){
                    ss.str("");
                    ss<<node[i];
                    see.insert(
                         LexicalCast<std::string,T>()(ss.str())
                    );
                }
                return see;
            }                

    };

    template<class T>
    class LexicalCast<std:: set<T>, std:: string>{
        public:
            std:: string operator() (const std::set<T>& v){
                YAML::Node node ;
                for(auto& i :v){
                    node.push_back(YAML:: Load( LexicalCast<T,std:: string>() (i) ) );
                }
                std:: stringstream ss;
                ss<<node;
                return ss.str();               
            }

    };

//  unordered_set
template<class T>
    class LexicalCast<std:: string,std:: unordered_set<T> >{
        public:
            std:: unordered_set<T> operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                typename std::unordered_set<T> see;
                std:: stringstream ss;
                for(size_t i=0;i<node.size();i++){
                    ss.str("");
                    ss<<node[i];
                    see.insert(
                         LexicalCast<std::string,T>()(ss.str())
                    );
                }
                return see;
            }                

    };

    template<class T>
    class LexicalCast<std:: unordered_set<T>, std:: string>{
        public:
            std:: string operator() (const std::unordered_set<T>& v){
                YAML::Node node ;
                for(auto& i :v){
                    node.push_back(YAML:: Load( LexicalCast<T,std:: string>() (i) ) );
                }
                std:: stringstream ss;
                ss<<node;
                return ss.str();               
            }

    };

//  map
template<class T>
    class LexicalCast<std:: string,std::map<std:: string, T> >{
        public:
            std:: map<std::string,T> operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                typename std::map<std::string,T> see;
                std:: stringstream ss;
                for(auto& i=node.begin();i!=node.end();i++){
                    ss.str("");
                    ss<<i->second;
                    see.insert(std::make_pair(i->first.Scalar() , LexicalCast<std::string,T>() ( ss.str() ) ) );
                }
                return see;
            }                

    };

    template<class T>
    class LexicalCast<std:: map<std:: string, T>, std:: string>{
        public:
            std:: string operator() (const std::map<std:: string, T>& v){
                YAML::Node node ;
                for(auto& i :v){
                    //node.push_back(YAML:: Load( LexicalCast<T,std:: string>() (i) ) );
                    node[i.first]=YAML:: Load( LexicalCast<T,std:: string>() (i.second)); 
                }
                std:: stringstream ss;
                ss<<node;
                return ss.str();               
            }

    };

//  unordermap
template<class T>
    class LexicalCast<std:: string,std::unordered_map<std:: string, T> >{
        public:
            std:: unordered_map<std::string,T> operator() (const std:: string& v){
                YAML::Node node = YAML:: Load(v);
                typename std::unordered_map<std::string,T> see;
                std:: stringstream ss;
                for(auto i=node.begin();i!=node.end();i++){
                    ss.str("");
                    ss<<i->second;
                    see.insert(std::make_pair(i->first.Scalar() , LexicalCast<std::string,T>() ( ss.str() ) ) );
                }
                return see;
            }                

    };

    template<class T>
    class LexicalCast<std:: unordered_map<std:: string, T>, std:: string>{
        public:
            std:: string operator() (const std::unordered_map<std:: string, T>& v){
                YAML::Node node ;
                for(auto& i :v){
                    //node.push_back(YAML:: Load( LexicalCast<T,std:: string>() (i) ) );
                    node[i.first]=YAML:: Load( LexicalCast<T,std:: string>() (i.second)); 
                }
                std:: stringstream ss;
                ss<<node;
                return ss.str();               
            }

    };

 






    template<class T, class FromStr=LexicalCast<std::string,T>, class ToStr=LexicalCast<T,std::string> >
    class ConfigVar: public ConfigVarBase{
        public:
            typedef std:: shared_ptr<ConfigVar> ptr;

            typedef std:: function<void (const T& old_value,const T& new_value) > on_change_cb;

            ConfigVar(const std:: string& name, const T& defaut_value,const std:: string& des=""):
            ConfigVarBase(name,des),m_val(defaut_value) {}
            std::string Tostring() override{
                try{
                    //return boost::lexical_cast<std:: string>(m_val);
                    return ToStr()(m_val);
                }
                catch(std:: exception& e){
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<" Convert to string exception "<<e.what()<<" convert "<<typeid(m_val).name()
                <<" to string ";

                }
                return "";
            }

            bool fromstring(const std:: string& val) override{
                try{
                   //m_val = boost::lexical_cast<T>(val);
                   setVal(FromStr()(val));
                   return true;
                }
                catch(std:: exception& e){
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<" string Convert to exception "<<e.what()<<
                    " convert string to"<<typeid(m_val).name();
            
                }
                return false;
            }

            const T getVal() const {return m_val;}

            void setVal(const T& val) {
                if(val==m_val){
                    return ;
                }
                else{
                    for(auto& i: m_cbs){
                        i.second(m_val,val);      //调用listener
                    }
                    m_val=val;
                }
            }

            std:: string getTypename() const override {return TypeToname<T>() ;}

            void addListener(uint64_t key, on_change_cb cb){
                m_cbs[key]=cb;
            }
            void deleListener(uint64_t key){
                m_cbs.erase(key);
            }
            on_change_cb getListener(uint64_t key){
                auto it=m_cbs.find(key);
                if(it==m_cbs.end()){
                    return nullptr;
                }
                return it->second;
            }



        private:
            T m_val;
            std:: map<u_int64_t,on_change_cb> m_cbs;
    };  

    class Config{
        public:
        typedef std:: map<std:: string,ConfigVarBase::ptr> ConfigVarMap;

        static ConfigVarMap& Getdatas(){
            static ConfigVarMap s_datas;          //和全局变量初始化的优先级是一样的
            return s_datas;
        }

        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std:: string& name,const T& defaut_value,const std:: string& des=""){
            auto it=Getdatas().find(name);
            if(it!=Getdatas().end()){
                auto tmp=std:: dynamic_pointer_cast<ConfigVar<T> > (it->second);
                if(tmp){
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"Lookup name ="<<name<<" exists";
                    return tmp;
                }
                else{
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"Lookup name ="<<name<<" exists but type not "<< TypeToname<T>() <<
                    "real type = "<<it->second->getTypename()<< " "<<it->second->Tostring();
                    return nullptr;
                }
            }

            if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"Lookup name invalid "<< name;
                throw std:: invalid_argument(name);
            }

            typename ConfigVar<T>:: ptr v(new ConfigVar<T>(name,defaut_value,des));
            Getdatas()[name]=v;
            return v;        
        }


        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std:: string& name){
            auto it=Getdatas().find(name);
            if(it==Getdatas().end()){
                return nullptr;  
            }
            return std:: dynamic_pointer_cast<ConfigVar<T> > (it->second);  //智能指针,从上到下转换  
        }

        static void LoadFromYaml(const YAML::Node& root);

        static ConfigVarBase:: ptr LookupConfBase(const  std:: string& str);

    };

}

#endif
