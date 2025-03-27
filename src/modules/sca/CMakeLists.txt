cmake_minimum_required(VERSION 3.22)

project(SCA)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

# find_package(Boost REQUIRED COMPONENTS asio)
# find_package(nlohmann_json REQUIRED)

add_library(SCA src/sca.cpp)
target_include_directories(SCA PUBLIC include)
# target_link_libraries(SCA PUBLIC SomePublicLibrary PRIVATE SomePrivateLibrary)

include(../../cmake/ConfigureTarget.cmake)
configure_target(ModuleManager)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
