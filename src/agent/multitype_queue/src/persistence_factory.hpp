#pragma once

#include <persistence.hpp>

#include <any>
#include <memory>
#include <string>
#include <vector>

class PersistenceFactory
{
public:
    static std::unique_ptr<Persistence> createPersistence(const std::string& type, const std::vector<std::any>& args);
};
