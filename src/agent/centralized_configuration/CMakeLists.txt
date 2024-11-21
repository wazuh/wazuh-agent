cmake_minimum_required(VERSION 3.22)

project(CentralizedConfiguration)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)
find_package(nlohmann_json REQUIRED)

add_library(CentralizedConfiguration src/centralized_configuration.cpp)
target_include_directories(CentralizedConfiguration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(CentralizedConfiguration PUBLIC ModuleCommand Boost::asio nlohmann_json::nlohmann_json Config PRIVATE Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(CentralizedConfiguration)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
