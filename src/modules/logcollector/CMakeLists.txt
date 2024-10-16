cmake_minimum_required(VERSION 3.22)

project(Logcollector)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

get_filename_component(SRC_FOLDER       ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER    ${SRC_FOLDER}/common/ ABSOLUTE)

include_directories(${COMMON_FOLDER}/logger/include)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

if (WIN32)
    FILE(GLOB EXCLUDED_SOURCES src/*_unix.cpp)
else()
    FILE(GLOB EXCLUDED_SOURCES src/*_win.cpp)
endif()

FILE(GLOB LOGCOLLECTOR_SOURCES src/*.cpp)
list(REMOVE_ITEM LOGCOLLECTOR_SOURCES ${EXCLUDED_SOURCES})

add_library(Logcollector ${LOGCOLLECTOR_SOURCES})

target_include_directories(Logcollector PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${COMMON_FOLDER})

target_link_libraries(Logcollector PUBLIC
    nlohmann_json::nlohmann_json
    ConfigurationParser
    MultiTypeQueue
    ModuleCommand
    Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logcollector)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
