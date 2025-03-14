cmake_minimum_required(VERSION 3.22)

project(hash_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(OpenSSL REQUIRED)

add_library(hash_helper src/hashHelper.cpp)

target_include_directories(hash_helper PUBLIC include)

target_link_libraries(hash_helper PRIVATE OpenSSL::Crypto)

include(../../cmake/ConfigureTarget.cmake)
configure_target(hash_helper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
