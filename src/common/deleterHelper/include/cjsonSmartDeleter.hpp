#pragma once

#include "customDeleter.hpp"
#include <cjson/cJSON.h>

struct CJsonSmartFree final : CustomDeleter<decltype(&cJSON_free), cJSON_free>
{
};

struct CJsonSmartDeleter final : CustomDeleter<decltype(&cJSON_Delete), cJSON_Delete>
{
};
