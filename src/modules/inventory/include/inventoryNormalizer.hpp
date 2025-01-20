#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <string>

class InvNormalizer
{
public:
    InvNormalizer(const std::string& configFile, const std::string& target);
    ~InvNormalizer() = default;
    void Normalize(const std::string& type, nlohmann::json& data) const;
    void RemoveExcluded(const std::string& type, nlohmann::json& data) const;

private:
    static std::map<std::string, nlohmann::json>
    GetTypeValues(const std::string& configFile, const std::string& target, const std::string& type);
    const std::map<std::string, nlohmann::json> m_typeExclusions;
    const std::map<std::string, nlohmann::json> m_typeDictionary;
};
