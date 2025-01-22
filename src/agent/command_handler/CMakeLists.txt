cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(CommandHandler)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

# CommandEntry target

find_package(nlohmann_json REQUIRED)

add_library(CommandEntry INTERFACE)

target_include_directories(CommandEntry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/command_entry)
target_link_libraries(CommandEntry INTERFACE nlohmann_json::nlohmann_json)

# CommandHandler target

find_package(Boost REQUIRED COMPONENTS asio)

add_library(CommandHandler src/command_handler.cpp src/command_store.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(CommandHandler)

target_include_directories(CommandHandler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(CommandHandler PUBLIC CommandEntry Boost::asio Logger PRIVATE Persistence)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
