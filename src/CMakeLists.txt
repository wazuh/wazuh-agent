cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
project(wazuh-agent)

add_subdirectory(external/rpm)

add_custom_target(
    build_procps_lib ALL
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${SRC_FOLDER}/src/external/procps
    COMMENT "Buiding procps library"
)

set(LIBDB_DIR "${SRC_FOLDER}/src/external/libdb")
add_custom_target(
    build_libdb ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Building libdb"
    COMMAND ${CMAKE_COMMAND} -E chdir ${LIBDB_DIR}/build_unix ${LIBDB_DIR}/dist/configure CFLAGS="-fPIC"
    COMMAND ${CMAKE_COMMAND} -E chdir ${LIBDB_DIR}/build_unix ${CMAKE_MAKE_PROGRAM}
    COMMENT "Building libdb"
)

add_subdirectory(agent)
add_subdirectory(common)
add_subdirectory(modules)

add_executable(wazuh-agent agent/src/main.cpp)
target_link_libraries(wazuh-agent Agent)

if(BUILD_TESTS)
    enable_testing()
endif()