cmake_minimum_required(VERSION 3.22)

project(OSLogStoreWrapper LANGUAGES CXX OBJCXX)

include(../../../../../cmake/CommonSettings.cmake)
set_common_settings()

if(NOT APPLE)
    message(FATAL_ERROR "OSLogStoreWrapper is only supported on Apple platforms")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(OSLogStoreWrapper ${CMAKE_CURRENT_SOURCE_DIR}/src/oslogstore.mm)
target_include_directories(OSLogStoreWrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(OSLogStoreWrapper PRIVATE
    Logger
    "-framework Foundation"
    "-framework OSLog"
)

include(../../../../../cmake/ConfigureTarget.cmake)
configure_target(OSLogStoreWrapper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
