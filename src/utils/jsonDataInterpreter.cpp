#include "jsonDataInterpreter.h"
#include <qdebug.h>

namespace jsonDataInterpreter{
    std::string getType(const json& information, json const* entry)
    {
        for(auto& [key, value] : information.items()){
            if(!value.is_array()){
                qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
                continue;
            }

            for(auto& arrayElement : value){
                if(arrayElement == *entry) return key;
            }
        }
        return "";
    }


    json& getNamed(json& target, const std::string& name, const std::string& type){
        for(auto& [key, value] : target.items()){
            if(!type.empty() && key != type) continue;
            if(!value.is_array()){
                qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
                continue;
            }

            for(auto& element : value){
                if(!element.contains("Name")) continue;
                std::vector<std::string> aliases = element["Name"].template get<std::vector<std::string>>();
                if(std::find(aliases.begin(), aliases.end(), name) != aliases.end()) return element;
            }
        }

        json& targetObject = target[type];
        if(!target.contains(type)) targetObject = json::array();
        json newObject = json::object();
        newObject["Name"] = json::array();
        newObject["Name"].push_back(name);
        targetObject.push_back(newObject);
        return targetObject.at(targetObject.size() - 1);
    }


    bool existsCoNamed(json& target, const std::string& name1, const std::string& name2,
            const std::string& type){
        for(auto& [key, value] : target.items()){
            if(!type.empty() && key != type) continue;
            if(!value.is_array()){
                qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
                continue;
            }

            for(auto& element : value){
                if(!element.contains("Name")) continue;
                std::vector<std::string> aliases = element["Name"].template get<std::vector<std::string>>();
                if(std::find(aliases.begin(), aliases.end(), name1) != aliases.end() &&
                        std::find(aliases.begin(), aliases.end(), name2) != aliases.end()) return true;
            }
        }
        return false;
    }

    std::vector<json*> getAllOfType(json& target, const std::string& type)
    {
        std::vector<json*> foundInstances;
        for(auto& [key, value] : target.items()){
            if(key != type) continue;

            if(!value.is_array()){
                qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
                continue;
            }
            for(auto& arrayElement : value){
                foundInstances.push_back(&arrayElement);
            }
        }
        return foundInstances;
    }
}
