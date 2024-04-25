#ifndef JSON_DATA_INTERPRETER_H
#define JSON_DATA_INTERPRETER_H

#include "json.hpp"
using json = nlohmann::json;

namespace jsonDataInterpreter{
    std::string getType(const json& information, json const* entry);
    json& getNamed(json& target, const std::string& name, const std::string& type = "");
    bool existsCoNamed(json& target, const std::string& name1, const std::string& name2,
            const std::string& type = "");
    std::vector<json*> getAllOfType(json& target, const std::string& type);
}

#endif //JSON_DATA_INTERPRETER_H
