#include "libyaml_wrapper.hpp"

YamlNode::YamlNode(yaml_document_t* doc, yaml_node_t* nodePtr)
    : m_document(doc)
    , m_node(nodePtr)
{
    if (!m_node)
    {
        return;
    }

    switch (m_node->type)
    {
        case YAML_SCALAR_NODE: m_type = Type::Scalar; break;
        case YAML_SEQUENCE_NODE: m_type = Type::Sequence; break;
        case YAML_MAPPING_NODE: m_type = Type::Mapping; break;
        case YAML_NO_NODE: m_type = Type::Undefined; break;
        default: m_type = Type::Undefined; break;
    }
}

void YamlNode::DumpYamlStructure(unsigned int indent) const
{
    std::string padding(indent, ' ');
    std::cout << padding << "Node Type: " << GetNodeTypeAsString();
    if (IsScalar())
    {
        std::cout << ", Value: " << AsString();
    }
    std::cout << std::endl;

    if (IsMap())
    {
        for (const auto& [key, val] : AsMap())
        {
            std::cout << padding << "- Key: " << key << std::endl;
            val.DumpYamlStructure(indent + 2);
        }
    }
    else if (IsSequence())
    {
        auto seq = AsSequence();
        for (size_t i = 0; i < seq.size(); ++i)
        {
            std::cout << padding << "- Index [" << i << "]" << std::endl;
            seq[i].DumpYamlStructure(indent + 2);
        }
    }
}

std::string YamlNode::AsString() const
{
    if (!IsScalar())
    {
        throw std::runtime_error("Node is not a scalar");
    }
    return std::string(reinterpret_cast<const char*>(m_node->data.scalar.value));
}

std::vector<YamlNode> YamlNode::AsSequence() const
{
    if (!IsSequence())
    {
        throw std::runtime_error("Node is not a sequence");
    }
    if (!sequence_cache.empty())
    {
        return sequence_cache;
    }

    for (yaml_node_item_t* item = m_node->data.sequence.items.start; item < m_node->data.sequence.items.top; ++item)
    {
        sequence_cache.emplace_back(m_document, yaml_document_get_node(m_document, *item));
    }
    return sequence_cache;
}

std::map<std::string, YamlNode> YamlNode::AsMap() const
{
    if (!IsMap())
    {
        throw std::runtime_error("Node is not a map");
    }
    if (!map_cache.empty())
    {
        return map_cache;
    }

    for (yaml_node_pair_t* pair = m_node->data.mapping.pairs.start; pair < m_node->data.mapping.pairs.top; ++pair)
    {
        auto key_node = yaml_document_get_node(m_document, pair->key);
        auto val_node = yaml_document_get_node(m_document, pair->value);
        if (key_node && key_node->type == YAML_SCALAR_NODE)
        {
            std::string key = reinterpret_cast<const char*>(key_node->data.scalar.value);
            map_cache[key] = YamlNode(m_document, val_node);
        }
    }
    return map_cache;
}

const YamlNode& YamlNode::operator[](const std::string& key) const
{
    if (!IsMap())
    {
        throw std::runtime_error("Not a map node");
    }
    auto map = AsMap();
    if (map.find(key) == map.end())
    {
        throw std::out_of_range("Key not found");
    }
    return map_cache[key];
}

const YamlNode& YamlNode::operator[](size_t index) const
{
    if (!IsSequence())
    {
        throw std::runtime_error("Not a sequence node");
    }
    auto seq = AsSequence();
    if (index >= seq.size())
    {
        throw std::out_of_range("Index out of bounds");
    }
    return sequence_cache[index];
}

YamlNode& YamlNode::operator[](const std::string& key)
{
    if (!IsMap())
    {
        throw std::runtime_error("Not a map node");
    }
    auto map = AsMap();
    if (map.find(key) == map.end())
    {
        throw std::out_of_range("Key not found");
    }
    return map_cache[key];
}

YamlNode& YamlNode::operator[](size_t index)
{
    if (!IsSequence())
    {
        throw std::runtime_error("Not a sequence node");
    }
    auto seq = AsSequence();
    if (index >= seq.size())
    {
        throw std::out_of_range("Index out of bounds");
    }
    return sequence_cache[index];
}

void YamlNode::SetScalarValue(const std::string& new_value)
{
    if (!IsScalar())
    {
        throw std::runtime_error("Not a scalar node");
    }

    // Free the old string
    free(m_node->data.scalar.value);

    // This new string will be owned and freed by the document on destruction
    auto newValue = strdup(new_value.c_str());

    m_node->data.scalar.value = reinterpret_cast<yaml_char_t*>(newValue);
    m_node->data.scalar.length = new_value.size();
}

bool YamlNode::HasKey(const std::string& key) const
{
    if (IsMap())
    {
        return AsMap().find(key) != AsMap().end();
    }
    else if (IsSequence())
    {
        auto seq = AsSequence();
        for (const auto& item : seq)
        {
            if (item.IsMap() && item.AsMap().find(key) != item.AsMap().end())
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

YamlDocument::YamlDocument(const std::filesystem::path& filename)
{
    std::ifstream file(filename.c_str());
    if (!file)
    {
        throw std::runtime_error("Cannot open file");
    };

    if (!Load(file))
    {
        // TODO: Log("Failed to parse YAML document:" filename.string().c_str());
    }
}

YamlDocument::YamlDocument(const std::string& yaml_content)
{
    std::istringstream ss(yaml_content);

    if (!Load(ss))
    {
        // TODO: Log("Failed to parse YAML content");
    }
}

YamlDocument::~YamlDocument()
{
    if (m_loaded)
    {
        yaml_document_delete(&m_document);
    }
    yaml_parser_delete(&m_parser);
}

bool YamlDocument::Load(std::istream& input)
{
    yaml_parser_initialize(&m_parser);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    yaml_parser_set_input_string(&m_parser, reinterpret_cast<const unsigned char*>(content.c_str()), content.size());
    m_loaded = yaml_parser_load(&m_parser, &m_document);

    return m_loaded;
}

YamlNode YamlDocument::GetRoot()
{
    auto root = yaml_document_get_root_node(&m_document);
    if (!root)
    {
        throw std::runtime_error("Empty YAML document");
    }
    return YamlNode(&m_document, root);
}
