cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(CommandStore)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(CommandStore src/command_store.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(CommandStore)

target_include_directories(CommandStore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(CommandStore PUBLIC ModuleCommand PRIVATE Persistence Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
