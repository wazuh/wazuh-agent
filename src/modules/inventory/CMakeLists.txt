cmake_minimum_required(VERSION 3.24)

project(Inventory)

get_filename_component(SRC_FOLDER       ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER    ${SRC_FOLDER}/common/ ABSOLUTE)

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
    ${COMMON_FOLDER}/debug_op/include
    ${COMMON_FOLDER}/hashHelper/include
    ${COMMON_FOLDER}/logging_helper/include
    ${COMMON_FOLDER}/pthreads_op/include
    ${COMMON_FOLDER}/stringHelper/include
    ${COMMON_FOLDER}/string_op/include
    ${COMMON_FOLDER}/timeHelper/include
    ${COMMON_FOLDER}/utils)

target_link_directories(inventory PUBLIC ${SRC_FOLDER}/external/pacman/lib/libalpm/)
target_link_directories(inventory PUBLIC ${SRC_FOLDER}/external/rpm/builddir/)
target_link_directories(inventory PUBLIC ${SRC_FOLDER}/external/libdb/build_unix/)

target_link_libraries(inventory dbsync sysinfo cjson nlohmann_json::nlohmann_json alpm rpm db)