#pragma once

namespace YAML
{
    class Node;
}

/// @brief Merges two YAML nodes, modifying the baseYaml node to include or additionalYaml values from the
/// additionalYaml node.
///
/// This function traverses the two YAML nodes. If a key exists in both nodes:
/// - If both values are maps, the function recurses to merge their content.
/// - If both values are sequences, their elements are concatenated.
/// - In all other cases (scalars, aliases, null values), the value from the additionalYaml node replaces the value in
/// the baseYaml node. If a key only exists in the additionalYaml node, it is added to the baseYaml node.
///
/// @param baseYaml Reference to the baseYaml YAML::Node that will be modified.
/// @param additionalYaml Const reference to the YAML::Node containing values to merge into the baseYaml.
void MergeYamlNodes(YAML::Node& baseYaml, const YAML::Node& additionalYaml);
