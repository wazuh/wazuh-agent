cmake_minimum_required(VERSION 3.22)

project(Communicator)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio url)
find_package(nlohmann_json REQUIRED)
find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

add_library(Communicator src/communicator.cpp)

target_include_directories(Communicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include SYSTEM
                           PRIVATE ${JWT_CPP_INCLUDE_DIRS})

target_compile_definitions(Communicator PRIVATE -DJWT_DISABLE_PICOJSON=ON)
target_link_libraries(Communicator PUBLIC HttpClient ConfigurationParser Boost::asio
                      PRIVATE Config Boost::url nlohmann_json::nlohmann_json Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Communicator)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
