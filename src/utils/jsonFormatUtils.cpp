#include "jsonFormatUtils.h"
#include <qdebug.h>

namespace jsonFormatUtils{
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

    json& createUnique(json& target, const std::string& inName, const std::string& type, unsigned int num){
        std::string name = num == 0 ? inName : inName + std::to_string(num);
        for(auto& [key, value] : target.items()){
            if(!type.empty() && key != type) continue;
            if(!value.is_array()) continue;

            for(auto& element : value){
                if(!element.contains("Name")) continue;
                std::vector<std::string> aliases = element["Name"].template get<std::vector<std::string>>();
                if(std::find(aliases.begin(), aliases.end(), name) == aliases.end()) continue;
                return createUnique(target, inName, type, num + 1);
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


    uint8_t getSupportedTypesIndex(const std::string& type){ 
#ifndef NDEBUG
        static bool asserted = false;
        if(!asserted){
            asserted = true;
            assertM(Stadt == getSupportedTypesIndex("Stadt") &&
                Region == getSupportedTypesIndex("Region") &&
                Bundesstaat == getSupportedTypesIndex("Bundesstaat") &&
                Staat == getSupportedTypesIndex("Staat") &&
                Halbinsel == getSupportedTypesIndex("Halbinsel") &&
                Insel == getSupportedTypesIndex("Insel") &&
                Fluss == getSupportedTypesIndex("Fluss") &&
                See == getSupportedTypesIndex("See") &&
                Ozean == getSupportedTypesIndex("Ozean") &&
                Meeresteil == getSupportedTypesIndex("Meeresteil") &&
                Kontinent == getSupportedTypesIndex("Kontinent") &&
                Gebirge == getSupportedTypesIndex("Gebirge") &&
                Gebirgskette == getSupportedTypesIndex("Gebirgskette"),
                "Not all type indices match their position in the SupportedTypes array");
        }
#endif

        for(int i = 0; i < supportedTypes.size(); i++){
            if(supportedTypes[i] != type) continue;
            return i;
        }
        
        //For Legacy Support
        if(type == "City") return Stadt | legacyFlag;
        if(type == "Continent") return Kontinent | legacyFlag;
        if(type == "Island") return Insel | legacyFlag;
        if(type == "IslandRegion") return Region | legacyFlag;
        if(type == "MountainRange") return Gebirgskette | legacyFlag;
        if(type == "Nation") return Staat | legacyFlag;
        if(type == "Ocean") return Ozean | legacyFlag;
        if(type == "Peninsula") return Halbinsel | legacyFlag;
        if(type == "River") return Fluss | legacyFlag;
        if(type == "Sea") return Meeresteil | legacyFlag;

        return uint8_t(-1);
    }

    QColor getAssociatedColor(uint8_t type){
        switch(type){
            case Stadt: return QColor(255, 20, 147);
            case Region: return QColor(255, 140, 0);
            case Bundesstaat: return QColor(199, 21, 133);
            case Staat: return QColor(220, 20, 60);
            case Halbinsel: return QColor(102,205,170);
            case Insel: return QColor(138, 43, 226);
            case Fluss: return QColor(135, 206, 250);
            case See: return QColor(0, 191, 255);
            case Ozean: return QColor(0, 0, 205);
            case Meeresteil: return QColor(30, 144, 255);
            case Kontinent: return QColor(255, 215, 0);
            case Gebirge: return QColor(47, 79, 79);
            case Gebirgskette: return QColor(255, 127, 80);
            default: return QColor(0, 0, 0);
        }
    }

std::string getWording(uint8_t type){
        switch(type){
            case Stadt: return "die Stadt, die";
            case Region: return "die Region, die";
            case Bundesstaat: return "der Bundesstaat, der";
            case Staat: return "der Staat, der";
            case Halbinsel: return "die Halbinsel, die";
            case Insel: return "die Insel, die";
            case Fluss: return "der Fluss, der";
            case See: return "der See, der";
            case Ozean: return "der Ozean, der";
            case Meeresteil: return "der Meeresteil, der";
            case Kontinent: return "der Kontinent, der";
            case Gebirge: return "das Gebirge, das";
            case Gebirgskette: return "die Gebirgskette, die";
            default:{
                        assertM(false, "Wording cannot be determined.");
                        return "__FEHLER__";
            }
        }
    }

    bool existsElementWithNameInTypeset(const json& root, const std::string& name, const std::vector<std::string>& typeset){
        bool exists = false; 
        for(const std::string& typeName : typeset){
            if(!root.contains(typeName)) continue;
            const json& type = root[typeName];
            if(!type.is_array()) continue;

            for(const json& element : type){
                if (!element.contains("Name")) continue;

                std::vector<std::string> names = element["Name"].template get<std::vector<std::string>>();    
                if(std::find(names.begin(), names.end(), name) == names.end()) continue;
                exists = true;
                break;
            }
        }

        return exists;
    }
}
