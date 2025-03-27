cmake_minimum_required(VERSION 3.22)

project(SCA)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(SCA src/sca.cpp)
target_include_directories(SCA PUBLIC include)
target_link_libraries(SCA PUBLIC dbsync Config)

include(../../cmake/ConfigureTarget.cmake)
configure_target(SCA)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
