cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(cJSON CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
<<<<<<< HEAD
=======
find_package(Lua REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
####################################################################################################
>>>>>>> d475bf20d (refactor: added dependencies required by data-provider)

project(wazuh-agent)

include(cmake/CommonSettings.cmake)
set_common_settings()

add_subdirectory(agent)
add_subdirectory(common/data_provider/)
add_subdirectory(modules)

add_executable(wazuh-agent agent/src/main.cpp)
target_link_libraries(wazuh-agent Agent)

include(cmake/ConfigureTarget.cmake)
configure_target(Agent)

if(BUILD_TESTS)
    enable_testing()
endif()

# Add the subdirectory for each child project
add_subdirectory(modules/)