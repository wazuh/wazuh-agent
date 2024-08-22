#pragma once

#include <string>

constexpr std::string_view CREATE_TABLE_QUERY {"CREATE TABLE IF NOT EXISTS {} (module TEXT, message TEXT NOT NULL);"};
