cmake_minimum_required(VERSION 3.24)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(OSLogStoreWrapper LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(OSLogStoreWrapper oslogstore_wrapper.mm)
target_include_directories(OSLogStoreWrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(OSLogStoreWrapper PRIVATE
    "-framework Foundation"
    "-framework OSLog"
)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    add_executable(oslogstore_wrapper_test oslogstore_wrapper_tests.cpp)
    target_include_directories(oslogstore_wrapper_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(oslogstore_wrapper_test PRIVATE
        OSLogStoreWrapper
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main)
    add_test(NAME OSLogStoreWrapperTest COMMAND oslogstore_wrapper_test)
endif()
