#include "libyaml_utils.hpp"

int yaml_parse_file(const char* path, yaml_document_t* document)
{
    yaml_parser_t parser;
    FILE* finput;
    int error = -1;

    if (finput = fopen(path, "rb"), finput)
    {
        yaml_parser_initialize(&parser);
        yaml_parser_set_input_file(&parser, finput);

        if (yaml_parser_load(&parser, document))
        {
            error = 0;
        }
        else
        {
            // mwarn("Failed to load YAML document in %s:%u", path, (unsigned int)parser.problem_mark.line);
            yaml_document_delete(document);
        }

        yaml_parser_delete(&parser);
        fclose(finput);
    }
    else
    {
        // mwarn("Cannot open file '%s': %s (%d)", path, strerror(errno), errno);
    }

    return error;
}
