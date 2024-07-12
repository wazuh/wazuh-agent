cmake_minimum_required(VERSION 3.12.4)

project(inventoryimp_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")


link_directories(${SRC_FOLDER}/shared_modules/dbsync/build/lib)
link_directories(${SRC_FOLDER}/shared_modules/rsync/build/lib)
include_directories(${SRC_FOLDER}/shared_modules/dbsync/src/)

file(GLOB INVENTORY_IMP_UNIT_TEST_SRC
    "*.cpp")

file(GLOB INVENTORY_IMP_SRC
    "${CMAKE_SOURCE_DIR}/src/inventoryImp.cpp"
    "${CMAKE_SOURCE_DIR}/src/inventoryNormalizer.cpp")

file(GLOB RSYNC_IMP_SRC
    "${SRC_FOLDER}/shared_modules/rsync/src/*.cpp")

file(GLOB DBSYNC_IMP_SRC
    "${SRC_FOLDER}/shared_modules/dbsync/src/*.cpp"
    "${SRC_FOLDER}/shared_modules/dbsync/src/sqlite/*.cpp")

add_definitions(-DWAZUH_UNIT_TESTING)

add_executable(inventoryimp_unit_test
    ${INVENTORY_IMP_UNIT_TEST_SRC}
    ${INVENTORY_IMP_SRC}
    ${RSYNC_IMP_SRC}
    ${DBSYNC_IMP_SRC})
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(inventoryimp_unit_test
        debug gtestd
        debug gmockd
        debug gtest_maind
        debug gmock_maind
        optimized gtest
        optimized gmock
        optimized gtest_main
        optimized gmock_main
        pthread
        cjson
        sqlite3
        crypto
        ws2_32
        ssl
        crypt32
        -static-libgcc -static-libstdc++
    )
else()
    target_link_libraries(inventoryimp_unit_test
        debug gtestd
        debug gmockd
        debug gtest_maind
        debug gmock_maind
        optimized gtest
        optimized gmock
        optimized gtest_main
        optimized gmock_main
        pthread
        sqlite3
        cjson
        crypto
        dl
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_test(NAME inventoryimp_unit_test
         COMMAND inventoryimp_unit_test)