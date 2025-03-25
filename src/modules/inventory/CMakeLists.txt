cmake_minimum_required(VERSION 3.22)

project(Inventory)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

add_library(Inventory
    src/inventory.cpp
    src/inventoryImp.cpp
    src/inventoryNormalizer.cpp
    src/statelessEvent.cpp)

target_include_directories(Inventory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Inventory
    PUBLIC
    ModuleManager
    ConfigurationParser
    MessageEntry
    CommandEntry
    dbsync
    sysinfo
    hash_helper
    string_helper
    time_helper
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::asio
    PRIVATE
    Config
    Logger
    cjson
)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Inventory)

add_subdirectory(testtool)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
