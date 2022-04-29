#ifndef __SYLAR_CONFIG_H_
#define __SYLAR_CONFIG_H_
#include<memory>
#include<string>
#include<boost/lexical_cast.hpp>
#include<sstream>
#include "log.h"
#include "util.h"
#include<yaml-cpp/yaml.h>

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

        private:
            std:: string m_name;
            std:: string m_descrip;
    };

    template<class T>
    class ConfigVar: public ConfigVarBase{
        public:
            typedef std:: shared_ptr<ConfigVar> ptr;
            ConfigVar(const std:: string& name, const T& defaut_value,const std:: string& des=""):
            ConfigVarBase(name,des),m_val(defaut_value) {}
            std::string Tostring() override{
                try{
                    return boost::lexical_cast<std:: string>(m_val);
                }
                catch(std:: exception& e){
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<" Convert to string exception "<<e.what()<<" convert "<<typeid(m_val).name()
                <<" to string ";

                }
                return "";
            }

            bool fromstring(const std:: string& val) override{
                try{
                   m_val = boost::lexical_cast<T>(val);
                   return true;
                }
                catch(std:: exception& e){
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<" string Convert to exception "<<e.what()<<
                    " convert string to"<<typeid(m_val).name();
            
                }
                return false;
            }

            std:: string getTypename() const override {return TypeToname<T>() ;}
        private:
            T m_val;
    };  

    class Config{
        public:
        typedef std:: map<std:: string,ConfigVarBase::ptr> ConfigVarMap;

        static ConfigVarMap& Getdatas(){
            static ConfigVarMap s_datas;
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
