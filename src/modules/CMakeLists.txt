cmake_minimum_required(VERSION 3.22)

project(ModuleManager)

include(../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)
find_package(nlohmann_json REQUIRED)

add_library(ModuleManager src/moduleManager.cpp)
target_include_directories(ModuleManager PUBLIC include)

message(STATUS "-------------------")
message(STATUS "Wazuh agent modules")
message(STATUS "-------------------")
message(STATUS "Inventory:      ${ENABLE_INVENTORY}")
message(STATUS "Logcollector:   ${ENABLE_LOGCOLLECTOR}")
message(STATUS "SCA:   ${ENABLE_SCA}")
message(STATUS "-------------------")

if(ENABLE_INVENTORY)
    add_subdirectory(inventory)
    target_link_libraries(ModuleManager PRIVATE Inventory)
endif()

if(ENABLE_LOGCOLLECTOR)
    add_subdirectory(logcollector)
    target_link_libraries(ModuleManager PRIVATE Logcollector)
endif()

if(ENABLE_SCA)
    add_subdirectory(sca)
    target_link_libraries(ModuleManager PRIVATE SCA)
endif()

target_link_libraries(ModuleManager PUBLIC Boost::asio nlohmann_json::nlohmann_json TaskManager ConfigurationParser MessageEntry CommandEntry PRIVATE Logger)

include(../cmake/ConfigureTarget.cmake)
configure_target(ModuleManager)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
