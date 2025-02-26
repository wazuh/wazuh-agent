cmake_minimum_required(VERSION 3.22)

project(RestartHandler)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(Boost REQUIRED COMPONENTS asio)

if(WIN32)
    set(SOURCES src/restart_handler.cpp src/restart_handler_win.cpp)
elseif(APPLE)
    set(SOURCES src/restart_handler.cpp src/restart_handler_unix.cpp src/restart_handler_mac.cpp)
else()
    set(SOURCES src/restart_handler.cpp src/restart_handler_unix.cpp src/restart_handler_lin.cpp)
endif()

add_library(RestartHandler ${SOURCES})
target_include_directories(RestartHandler
                            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(RestartHandler PUBLIC Boost::asio CommandEntry PRIVATE Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(RestartHandler)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
