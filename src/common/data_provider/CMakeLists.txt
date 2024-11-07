cmake_minimum_required(VERSION 3.22)

project(sysinfo)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

enable_testing()

get_filename_component(SRC_FOLDER     ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)

if(COVERITY)
  add_definitions(-D__GNUC__=8)
endif(COVERITY)

include(CheckCXXSourceCompiles)
set(CMAKE_CXX_STANDARD 17)
#Check if std::optional is available, therefore C++17 is available
check_cxx_source_compiles("
    #include <optional>
    int main() {
        std::optional<int> o;
        return 0;
    }
" HAS_OPTIONAL)

if(NOT HAS_OPTIONAL AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CMAKE_CXX_STANDARD 14)
endif()

#Check if filesystem is available
check_cxx_source_compiles("
    #include <filesystem>
    int main() {
        std::filesystem::path p;
        return 0;
    }
" HAS_FILESYSTEM)

if(HAS_FILESYSTEM OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(-DHAS_STDFILESYSTEM=true)
else()
  add_definitions(-DHAS_STDFILESYSTEM=false)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wunused -Wcast-align -Wformat=2")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
  endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

  if(FSANITIZE)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,leak,undefined")
  endif(FSANITIZE)
else()
  set(CMAKE_CXX_FLAGS "/W4 /permissive- /MT")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

find_package(cJSON CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Lua REQUIRED)
if(APPLE)
  find_package(unofficial-libplist CONFIG REQUIRED)
endif()
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

find_path(POPT_INCLUDE_DIR include/popt.h)
include_directories(${POPT_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${SRC_FOLDER}/headers/)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SRC_FOLDER}/common/)
include_directories(${SRC_FOLDER}/common/utils/)
include_directories(${SRC_FOLDER}/common/byteArrayHelper/include/)
include_directories(${SRC_FOLDER}/common/cmdHelper/include/)
include_directories(${SRC_FOLDER}/common/globHelper/include/)
include_directories(${SRC_FOLDER}/common/fileHelper/include/)
include_directories(${SRC_FOLDER}/common/jsonHelper/include/)
include_directories(${SRC_FOLDER}/common/linuxHelper/include/)
include_directories(${SRC_FOLDER}/common/stringHelper/include/)
include_directories(${SRC_FOLDER}/common/timeHelper/include/)
include_directories(${SRC_FOLDER}/shared_modules/common/)

if(WIN32)
  include_directories(${SRC_FOLDER}/common/time_op/include/)
  include_directories(${SRC_FOLDER}/common/file_op/include/)
  include_directories(${SRC_FOLDER}/common/regex_op/include/)
  include_directories(${SRC_FOLDER}/common/bzip2_op/include/)
  include_directories(${SRC_FOLDER}/common/validate_op/include/)
  include_directories(${SRC_FOLDER}/common/binaries_op/include/)
  include_directories(${SRC_FOLDER}/common/randombytes/include/)
  include_directories(${SRC_FOLDER}/common/error_messages/include/)
  include_directories(${SRC_FOLDER}/common/registryHelper/include/)
  include_directories(${SRC_FOLDER}/common/windowsHelper/include/)
  include_directories(${SRC_FOLDER}/common/encodingHelper/include/)
  include_directories(${SRC_FOLDER}/common/mem_op/include/)
endif()

get_filename_component(COMMON_FOLDER  ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
include_directories(${COMMON_FOLDER}/pal/include)

link_directories(${SRC_FOLDER})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  file(GLOB SYSINFO_SRC
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*Win.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsInfoWin.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*Windows.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/*Windows.cpp")
  add_definitions(-DWIN32=1
                  -D_WIN32_WINNT=0x600)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  file(GLOB SYSINFO_SRC
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*Linux.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*Linux.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/packageLinux*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/rpm*.cpp")
  add_definitions(-DLINUX_TYPE=LinuxType::STANDARD) # Standard compilation in compatible systems
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64.*|ARM64.*")
    file(GLOB SYSINFO_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*Mac.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*CommonBSD.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/*Mac.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*BSD.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/hardware/*ARMMac.cpp")
  else()
    file(GLOB SYSINFO_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*Mac.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*CommonBSD.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/packages/*Mac.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*BSD.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/hardware/*X86_64Mac.cpp")
  endif()
else()
  file(GLOB SYSINFO_SRC
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*Unix.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/osinfo/sysOsParsers.cpp")
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_library(sysinfo STATIC
    ${SYSINFO_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sysInfo.cpp
    ${SRC_FOLDER}/${RESOURCE_OBJ})

target_include_directories(sysinfo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(sysinfo PUBLIC psapi iphlpapi ws2_32)
elseif(APPLE)
  find_library(iokit_lib IOKit)
  if(NOT iokit_lib)
    message(FATAL_ERROR "IOKit library not found! Aborting...")
  endif()
  find_library(corefoundation_lib CoreFoundation)
  if(NOT corefoundation_lib)
    message(FATAL_ERROR "CoreFoundation library not found! Aborting...")
  endif()

  target_link_libraries(sysinfo PUBLIC
    unofficial::libplist::libplist
    unofficial::libplist::libplist++
    ${iokit_lib}
    ${corefoundation_lib}
    sqliteWrapper
    LibArchive::LibArchive)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

target_link_libraries(sysinfo PUBLIC
  networkHelper
  nlohmann_json::nlohmann_json
  cjson)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(POPT_LIBRARY libpopt.a)
  find_library(PROC_NG_LIBRARY NAMES proc-ng REQUIRED)
  find_library(LIBDB_LIBRARY NAMES db REQUIRED)
  find_library(LIBRPM_LIBRARY NAMES rpm REQUIRED)
  find_library(LIBRPMIO_LIBRARY NAMES rpmio REQUIRED)
  target_link_libraries(sysinfo PUBLIC
    ${LIBDB_LIBRARY}
    ${PROC_NG_LIBRARY}
    ${LIBRPM_LIBRARY}
    ${LIBRPMIO_LIBRARY}
    LibArchive::LibArchive
    ${POPT_LIBRARY}
    ${LUA_LIBRARIES}
  )
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

target_link_libraries(sysinfo PUBLIC
  unofficial::sqlite3::sqlite3)

if(BUILD_TESTS)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_link_libraries(sysinfo PRIVATE -fprofile-arcs)
    else()
      target_link_libraries(sysinfo PRIVATE gcov)
    endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    add_subdirectory(tests)
  endif(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
else()
  if(FSANITIZE)
      target_link_libraries(sysinfo PRIVATE gcov)
  endif(FSANITIZE)
  add_subdirectory(testtool)
endif(BUILD_TESTS)
