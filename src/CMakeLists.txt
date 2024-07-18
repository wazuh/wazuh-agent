cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
project(wazuh-agent)

include(cmake/CommonSettings.cmake)
set_common_settings()

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_custom_target(external_deps ALL
        COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Downloading external dependencies"
    )
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX-")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX-")
    add_compile_options(/w)
endif()

add_subdirectory(common)
add_subdirectory(modules)
add_subdirectory(agent)
add_subdirectory(common/data_provider/)
add_subdirectory(modules)

add_executable(wazuh-agent agent/src/main.cpp)
target_link_libraries(wazuh-agent Agent Logger)

include(cmake/ConfigureTarget.cmake)
configure_target(Agent)

if(BUILD_TESTS)
    enable_testing()
endif()