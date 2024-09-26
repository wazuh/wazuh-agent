cmake_minimum_required(VERSION 3.22)

project(Inventory)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

get_filename_component(SRC_FOLDER       ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER    ${SRC_FOLDER}/common/ ABSOLUTE)

include_directories(${COMMON_FOLDER}/logger/include)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(Inventory
    src/inventory.cpp
    src/inventoryImp.cpp
    src/inventoryNormalizer.cpp)

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
    ${COMMON_FOLDER}/utils)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_directories(Inventory PUBLIC ${SRC_FOLDER}/external/rpm/builddir/)
endif()
target_link_directories(Inventory PUBLIC ${SRC_FOLDER}/external/libdb/build_unix/)
target_link_directories(Inventory PUBLIC ${SRC_FOLDER}/external/procps/)

target_link_libraries(Inventory PUBLIC
    dbsync
    sysinfo
    cjson
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    utils
    pthreads_op
    ConfigurationParser
    MultiTypeQueue
    Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Inventory)

add_subdirectory(testtool)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
