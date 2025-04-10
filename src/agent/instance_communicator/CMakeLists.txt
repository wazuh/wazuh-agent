cmake_minimum_required(VERSION 3.22)

project(InstanceCommunicator)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

if(WIN32)
    set(SOURCES src/instance_communicator.cpp src/instance_communicator_win.cpp)
else()
    set(SOURCES src/instance_communicator.cpp src/instance_communicator_unix.cpp)
endif()

add_library(InstanceCommunicator ${SOURCES})
target_include_directories(InstanceCommunicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(InstanceCommunicator PUBLIC Boost::asio PRIVATE Logger Config)

include(../../cmake/ConfigureTarget.cmake)
configure_target(InstanceCommunicator)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
