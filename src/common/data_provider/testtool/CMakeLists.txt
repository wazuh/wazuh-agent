cmake_minimum_required(VERSION 3.22)

project(sysinfo_test_tool)

include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${SRC_FOLDER}/shared_modules/common/)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wunused -Wcast-align -Wformat=2 -std=c++14")
  if(FSANITIZE)
    set(CMAKE_CXX_FLAGS_DEBUG "-pthread -fsanitize=address,leak,undefined")
  endif(FSANITIZE)
else()
  set(CMAKE_CXX_FLAGS "/W4 /permissive- /std:c++14 /MT /EHsc")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DWIN32=1)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_executable(sysinfo_test_tool
               ${sysinfo_TESTTOOL_SRC}
               ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(sysinfo_test_tool
        sysinfo
        psapi
        iphlpapi
        ws2_32
    )
else()
    target_link_libraries(sysinfo_test_tool
        sysinfo
        dl
        pthread
    )
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

if(APPLE)
  add_custom_command(TARGET sysinfo_test_tool
    POST_BUILD COMMAND
    ${CMAKE_INSTALL_NAME_TOOL} -change "@rpath/libsysinfo.dylib" "@executable_path/../lib/libsysinfo.dylib"
    $<TARGET_FILE:sysinfo_test_tool>)
add_custom_command(TARGET sysinfo_test_tool
    POST_BUILD COMMAND
    ${CMAKE_INSTALL_NAME_TOOL} -change "@rpath/libwazuhext.dylib" "@executable_path/../../../libwazuhext.dylib"
    $<TARGET_FILE:sysinfo_test_tool>)
endif(APPLE)
