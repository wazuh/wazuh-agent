cmake_minimum_required(VERSION 3.24)

project(Inventory)

get_filename_component(SRC_FOLDER       ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER    ${SRC_FOLDER}/common/ ABSOLUTE)

find_package(cJSON CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(inventory
    src/inventory.cpp
    src/inventoryImp.cpp
    src/inventoryNormalizer.cpp)

target_include_directories(inventory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${COMMON_FOLDER}
    ${COMMON_FOLDER}/data_provider/include
    ${COMMON_FOLDER}/dbsync/include
    ${COMMON_FOLDER}/error_messages/include
    ${COMMON_FOLDER}/file_op/include
    ${COMMON_FOLDER}/hashHelper/include
    ${COMMON_FOLDER}/logging_helper/include
    ${COMMON_FOLDER}/os_xml/include
    ${COMMON_FOLDER}/privsep_op/include
    ${COMMON_FOLDER}/pthreads_op/include
    ${COMMON_FOLDER}/stringHelper/include
    ${COMMON_FOLDER}/string_op/include
    ${COMMON_FOLDER}/time_op/include
    ${COMMON_FOLDER}/timeHelper/include
    ${COMMON_FOLDER}/utils)

target_link_directories(inventory PUBLIC ${SRC_FOLDER}/external/rpm/builddir/)
target_link_directories(inventory PUBLIC ${SRC_FOLDER}/external/libdb/build_unix/)
target_link_directories(inventory PUBLIC ${SRC_FOLDER}/external/procps/)


target_link_libraries(inventory PUBLIC
    dbsync
    sysinfo
    cjson
    nlohmann_json::nlohmann_json
    rpm
    db
    proc
    utils
    os_regex
    logging_helper
    pthreads_op)