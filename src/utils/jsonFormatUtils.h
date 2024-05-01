#ifndef JSON_DATA_INTERPRETER_H
#define JSON_DATA_INTERPRETER_H

#include "json.hpp"
#include <qcolor.h>
#include <iostream> //required for the assertM macro
using json = nlohmann::json;

#ifndef NDEBUG
#   define assertM(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::abort(); \
        } \
    } while (false)
#else
#   define assertM(condition, message) do { } while (false)
#endif

#define SUPPORTED_TYPES "Stadt", "Region", "Bundesstaat", "Staat", "Halbinsel", "Insel", "Fluss", "See", "Ozean", "Meeresteil", "Kontinent", "Gebirge", "Gebirgskette"
#define LEGACY_SUPPORTED_TYPES "City", "Continent", "Island", "IslandRegion", "MountainRange", "Nation", "Ocean", "Peninsula", "River", "Sea"


namespace jsonFormatUtils{
    enum SupportedTypes : uint8_t {
        Stadt,
        Region,
        Bundesstaat,
        Staat,
        Halbinsel,
        Insel,
        Fluss,
        See,
        Ozean,
        Meeresteil,
        Kontinent,
        Gebirge,
        Gebirgskette
    };

    std::string getType(const json& information, json const* entry);
    json& getNamed(json& target, const std::string& name, const std::string& type = "");
    json& createUnique(json& target, const std::string& name, const std::string& type, unsigned int num = 0);
    bool existsCoNamed(json& target, const std::string& name1, const std::string& name2,
            const std::string& type = "");
    std::vector<json*> getAllOfType(json& target, const std::string& type);

    static const std::vector<std::string> supportedTypes({SUPPORTED_TYPES});
    inline const uint8_t legacyFlag = 0b10000000;
    uint8_t getSupportedTypesIndex(const std::string& type);
    QColor getAssociatedColor(uint8_t typeIndex);
    std::string getWording(uint8_t typeIndex);
}

#endif //JSON_DATA_INTERPRETER_H
