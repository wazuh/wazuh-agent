cmake_minimum_required(VERSION 3.22)

project(CentralizedConfiguration)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

include_directories(${CMAKE_SOURCE_DIR}/common/logger/include)

add_library(CentralizedConfiguration src/centralized_configuration.cpp)
target_include_directories(CentralizedConfiguration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(CentralizedConfiguration PUBLIC ModuleCommand Boost::asio)

include(../../cmake/ConfigureTarget.cmake)
configure_target(CentralizedConfiguration)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
