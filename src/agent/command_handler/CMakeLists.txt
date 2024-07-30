cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(CommandHandler)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(CommandHandler src/command_handler.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(CommandHandler)

target_include_directories(CommandHandler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../command_store/include)
target_link_libraries(CommandHandler PUBLIC CommandStore)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
