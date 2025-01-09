cmake_minimum_required(VERSION 3.22)

project(Restart)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)
find_package(nlohmann_json REQUIRED)

add_library(Restart src/restart.cpp)
target_include_directories(Restart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Restart PUBLIC ModuleCommand Boost::asio nlohmann_json::nlohmann_json PRIVATE Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Restart)

# if(BUILD_TESTS)
#     enable_testing()
#     add_subdirectory(tests)
# endif()
