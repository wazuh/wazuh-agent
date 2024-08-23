#pragma once

#include <string>

constexpr std::string_view CREATE_TABLE_QUERY {
    "CREATE TABLE IF NOT EXISTS {} (module TEXT, message TEXT NOT NULL);"}; // params: tableName
constexpr std::string_view INSERT_QUERY {
    "INSERT INTO {} (module, message) VALUES (\"{}\", \"{}\");"}; // params: tableName, moduleName, value
constexpr std::string_view INSERT_QUERY_BIND {
    "INSERT INTO {} (module, message) VALUES (\"{}\", ?);"}; // params: tableName, moduleName, json object
