cmake_minimum_required(VERSION 3.22)

project(InstanceCommunicator)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

if(WIN32)
    set(SOURCES src/instance_communicator.cpp src/listener_wrapper_win.cpp src/pipe/pipe_wrapper.cpp)
else()
    set(SOURCES src/instance_communicator.cpp src/listener_wrapper_unix.cpp src/socket/socket_wrapper.cpp)
endif()

add_library(InstanceCommunicator ${SOURCES})

if(WIN32)
    target_include_directories(InstanceCommunicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                               PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/pipe)
    target_compile_definitions(InstanceCommunicator PRIVATE USE_PIPE_WRAPPER)
else()
    target_include_directories(InstanceCommunicator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                               PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/socket)
    target_compile_definitions(InstanceCommunicator PRIVATE USE_SOCKET_WRAPPER)
endif()

target_link_libraries(InstanceCommunicator PUBLIC Boost::asio PRIVATE Logger Config)

include(../../cmake/ConfigureTarget.cmake)
configure_target(InstanceCommunicator)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
