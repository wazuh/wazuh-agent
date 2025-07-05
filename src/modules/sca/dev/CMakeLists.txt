cmake_minimum_required(VERSION 3.22)
project(YamlWrappertest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

find_package(yaml REQUIRED)

include_directories(${yaml_INCLUDE_DIRS})
link_directories(${YAML_LIBRARY_DIRS})

add_executable(yamltest libyaml_test.cpp ../src/libyaml_wrapper.cpp)

target_link_libraries(yamltest PRIVATE yaml)
