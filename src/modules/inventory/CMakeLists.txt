cmake_minimum_required(VERSION 3.22)

project(Inventory)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

get_filename_component(SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER ${SRC_FOLDER}/common/ ABSOLUTE)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)

add_library(Inventory
    src/inventory.cpp
    src/inventoryImp.cpp
    src/inventoryNormalizer.cpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Inventory PRIVATE /WX- /w)
endif()

target_include_directories(Inventory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${COMMON_FOLDER}
    ${COMMON_FOLDER}/data_provider/include
    ${COMMON_FOLDER}/dbsync/include
    ${COMMON_FOLDER}/error_messages/include
    ${COMMON_FOLDER}/file_op/include
    ${COMMON_FOLDER}/hashHelper/include
    ${COMMON_FOLDER}/privsep_op/include
    ${COMMON_FOLDER}/pthreads_op/include
    ${COMMON_FOLDER}/stringHelper/include
    ${COMMON_FOLDER}/time_op/include
    ${COMMON_FOLDER}/timeHelper/include
    ${COMMON_FOLDER}/utils
    ${COMMON_FOLDER}/pal/include)

target_link_libraries(Inventory
    PUBLIC
    Config
    dbsync
    sysinfo
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    utils
    pthreads_op
    ConfigurationParser
    MultiTypeQueue
    ModuleCommand
    Boost::asio
    Boost::beast
    PRIVATE
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
