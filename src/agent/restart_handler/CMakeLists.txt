cmake_minimum_required(VERSION 3.22)

project(RestartHandler)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

if(WIN32)
    set(SOURCES src/restart_handler_win.cpp)
else()
    set(SOURCES src/restart_handler_unix.cpp)
endif()

add_library(RestartHandler ${SOURCES})
target_include_directories(RestartHandler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(RestartHandler PUBLIC Boost::asio CommandEntry PRIVATE Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(RestartHandler)
