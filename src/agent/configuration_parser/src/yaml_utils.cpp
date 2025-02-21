#include "yaml_utils.hpp"

#include <yaml-cpp/yaml.h>

#include <queue>
#include <unordered_set>

void MergeYamlNodes(YAML::Node& baseYaml, const YAML::Node& additionalYaml)
{
    // Queue to manage nodes to be merged. Pairs of nodes are handled directly.
    std::queue<std::pair<YAML::Node, YAML::Node>> nodesToProcess;
    nodesToProcess.emplace(baseYaml, additionalYaml);

    while (!nodesToProcess.empty())
    {
        auto [baseNode, additionalNode] = nodesToProcess.front();
        nodesToProcess.pop();

        // Traverse each key-value pair in the additionalYaml node.
        for (auto it = additionalNode.begin(); it != additionalNode.end(); ++it)
        {
            const auto key = it->first.as<std::string>();
            const YAML::Node value = it->second;

            if (baseNode[key])
            {
                // Key exists in the baseYaml node.
                if (value.IsMap() && baseNode[key].IsMap())
                {
                    // Both values are maps: enqueue for further merging.
                    nodesToProcess.emplace(baseNode[key], value);
                }
                else if (value.IsSequence() && baseNode[key].IsSequence())
                {
                    // Merge sequences while preserving the order.
                    YAML::Node mergedSequence = YAML::Node(YAML::NodeType::Sequence);

                    // Collect elements from 'additionalYaml' sequence to preserve insertion order.
                    std::vector<std::pair<std::string, YAML::Node>> additionalElements;
                    for (const YAML::Node& elem : value)
                    {
                        if (elem.IsScalar())
                        {
                            additionalElements.emplace_back(elem.as<std::string>(), elem);
                        }
                        else if (elem.IsMap() && elem.begin() != elem.end())
                        {
                            additionalElements.emplace_back(elem.begin()->first.as<std::string>(), elem);
                        }
                    }

                    // Track which keys from 'additionalYaml' sequence are merged.
                    std::unordered_set<std::string> mergedKeys;

                    for (const YAML::Node& elem : baseNode[key])
                    {
                        std::string elemKey;

                        // Extract the key based on the type of element.
                        if (elem.IsScalar())
                        {
                            elemKey = elem.as<std::string>();
                        }
                        else if (elem.IsMap() && elem.begin() != elem.end())
                        {
                            elemKey = elem.begin()->first.as<std::string>();
                        }
                        else
                        {
                            // Skip elements that don't fit the expected types.
                            mergedSequence.push_back(elem);
                            continue;
                        }

                        // Common logic for merging elements.
                        auto additionalItem =
                            std::find_if(additionalElements.begin(),
                                         additionalElements.end(),
                                         [&elemKey](const auto& pair) { return pair.first == elemKey; });
                        if (additionalItem != additionalElements.end())
                        {
                            mergedSequence.push_back(additionalItem->second);
                            mergedKeys.insert(additionalItem->first);
                        }
                        else
                        {
                            mergedSequence.push_back(elem);
                        }
                    }

                    // Add remaining elements from 'additionalYaml' sequence in order.
                    for (const auto& [itemKey, itemNode] : additionalElements)
                    {
                        if (mergedKeys.find(itemKey) == mergedKeys.end())
                        {
                            mergedSequence.push_back(itemNode);
                        }
                    }

                    baseNode[key] = mergedSequence;
                }
                else
                {
                    // Other cases (scalar, alias, null): overwrite the value.
                    baseNode[key] = value;
                }
            }
            else
            {
                // Key does not exist in the baseYaml node: add it directly.
                baseNode[key] = value;
            }
        }
    }
}
