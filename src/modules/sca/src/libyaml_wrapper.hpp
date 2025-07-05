// yaml_wrapper.hpp
#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <yaml.h>

class YamlNode;

class YamlNode
{
public:
    enum class Type
    {
        Scalar,
        Sequence,
        Mapping,
        Undefined
    };

    YamlNode(yaml_document_t* doc, yaml_node_t* node);
    YamlNode() = default;

    Type GetNodeType() const
    {
        return m_type;
    }

    std::string GetNodeTypeAsString() const
    {
        switch (m_type)
        {
            case Type::Scalar: return "Scalar";
            case Type::Sequence: return "Sequence";
            case Type::Mapping: return "Mapping";
            case Type::Undefined: // fallthrough
            default: return "Undefined";
        }
    }

    bool IsScalar() const
    {
        return m_type == Type::Scalar;
    }

    bool IsSequence() const
    {
        return m_type == Type::Sequence;
    }

    bool IsMap() const
    {
        return m_type == Type::Mapping;
    }

    std::string AsString() const;
    std::vector<YamlNode> AsSequence() const;
    std::map<std::string, YamlNode> AsMap() const;

    const YamlNode& operator[](const std::string& key) const;
    const YamlNode& operator[](size_t index) const;

    YamlNode& operator[](const std::string& key);
    YamlNode& operator[](size_t index);

    void SetScalarValue(const std::string& new_value);

    bool HasKey(const std::string& key) const;

    //@brief Dump the YAML structure to the console for debugging purposes
    void DumpYamlStructure(unsigned int indent = 2) const;

private:
    yaml_document_t* m_document = nullptr;
    yaml_node_t* m_node = nullptr;
    Type m_type = Type::Undefined;
    mutable std::map<std::string, YamlNode> map_cache;
    mutable std::vector<YamlNode> sequence_cache;
    mutable std::string scalar_cache;
};

class YamlDocument
{
public:
    YamlDocument(const std::filesystem::path& filename);
    YamlDocument(const std::string& yaml_content);
    ~YamlDocument();

    YamlNode GetRoot();

    bool IsValidDocument() const
    {
        return m_loaded;
    }

private:
    yaml_parser_t m_parser;
    yaml_document_t m_document;
    bool m_loaded = false;

    bool Load(std::istream& input);
};
