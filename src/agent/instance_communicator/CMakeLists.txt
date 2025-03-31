cmake_minimum_required(VERSION 3.22)

project(InstanceCommunicator)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

add_library(InstanceCommunicator src/instance_communicator.cpp)
target_include_directories(InstanceCommunicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(InstanceCommunicator PUBLIC Boost::asio PRIVATE Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(InstanceCommunicator)

# if(BUILD_TESTS) enable_testing() add_subdirectory(tests) endif()
