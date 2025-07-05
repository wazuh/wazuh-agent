#pragma once

#include <yaml.h>

int yaml_parse_file(const char* path, yaml_document_t* document);
