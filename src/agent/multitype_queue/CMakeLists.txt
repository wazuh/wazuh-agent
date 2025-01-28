cmake_minimum_required(VERSION 3.22)

project(MultiTypeQueue)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

# MessageEntry target

find_package(nlohmann_json REQUIRED)

add_library(MessageEntry INTERFACE)

target_include_directories(MessageEntry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/message_entry)
target_link_libraries(MessageEntry INTERFACE nlohmann_json::nlohmann_json)

# MultiTypeQueue target

find_package(Boost REQUIRED COMPONENTS asio)

add_library(MultiTypeQueue src/storage.cpp src/multitype_queue.cpp)

target_include_directories(MultiTypeQueue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(MultiTypeQueue
    PUBLIC
    Config
    ConfigurationParser
    MessageEntry
    Boost::asio
    PRIVATE
    Persistence
    Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(MultiTypeQueue)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
