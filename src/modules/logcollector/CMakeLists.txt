cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Logcollector)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_filename_component(SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER ${SRC_FOLDER}/common/ ABSOLUTE)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio system)

if(WIN32)
    FILE(GLOB EXCLUDED_SOURCES src/*_unix.cpp src/*_osx.cpp src/journal*.cpp)
elseif(APPLE)
    FILE(GLOB EXCLUDED_SOURCES src/*_win.cpp src/journal*.cpp)
else()
    FILE(GLOB EXCLUDED_SOURCES src/*_win.cpp src/*_osx.cpp)
endif()

FILE(GLOB LOGCOLLECTOR_SOURCES src/*.cpp)
list(REMOVE_ITEM LOGCOLLECTOR_SOURCES ${EXCLUDED_SOURCES})

add_library(Logcollector ${LOGCOLLECTOR_SOURCES})

target_include_directories(Logcollector PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${COMMON_FOLDER}
    ${COMMON_FOLDER}/stringHelper/include
    ${COMMON_FOLDER}/timeHelper/include)

target_link_libraries(Logcollector
    PUBLIC
    ConfigurationParser
    MultiTypeQueue
    ModuleCommand
    Config
    Boost::asio
    Boost::system
    nlohmann_json::nlohmann_json
    $<$<PLATFORM_ID:Linux>:systemd>
    PRIVATE
    Logger
    pal
)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logcollector)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
